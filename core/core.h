#include "constant.h"
#include "helper.h"
#include "ui.h"

static struct shm_info *INPUT;
static char *input_buf;
static sem_t *input_sem;

static struct shm_info *PTABLE;
static struct proc_table *ptable;
static sem_t *ptable_sem;

void isolate_proc() {
    pid_t pid = getpid();
    if (setpgid(pid, pid) == -1) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
}

// Assure parent start after child signals
void spawn_proc(void (*child)(), void (*parent)(), void *arg) {
    struct shm_info shmem = create_shm(SHM_ORDER, sizeof(int));
    int *order = (int *)shmem.data;
    *order = ORDER_CHILD;
    pid_t pid = fork();

    if (pid < 0) {
        perror("Initialize Error");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        isolate_proc();
        *order = ORDER_PARENT;
        kill(pid, SIGTSTP);

        if (child)
            child(arg);
    } else {
        while (*order == ORDER_PARENT);
        reptyr_proc(pid);

        if (parent)
            parent(pid, arg);
    }

    destroy_shm(&shmem);
}

void parse_cmd(char *cmd, char **args) {
    while (*cmd != '\0') {
        while (isspace(*cmd)) cmd++;  // 공백 건너뛰기

        if (*cmd == '\"' || *cmd == '\'') {  // 따옴표 시작을 찾으면
            char quote = *cmd++;  // 따옴표 기억
            *args++ = cmd;  // 따옴표 다음 문자를 인자 시작으로 설정
            while (*cmd && *cmd != quote) cmd++;  // 다음 따옴표 찾기
        } else {
            *args++ = cmd;  // 현재 위치를 인자 시작으로 설정
            while (*cmd && !isspace(*cmd) && *cmd != '\"' && *cmd != '\'') cmd++;  // 인자 끝 찾기
        }
        
        if (*cmd) *cmd++ = '\0';  // 현재 인자를 끝내고 다음 인자로
    }
    *args = NULL;  // 인자 리스트 종료
}

void os_spawn(char *cmd) {
    char *args[BUF_SZ];
    int ret;

    parse_cmd(cmd, args);
    ret = execvp(args[0], args);
    if (ret != 0) {
        printf("'%s' %s\n", cmd, strerror(errno));
    }
}

void register_proc(pid_t pid, char *cmd) {
    sem_wait(ptable_sem);
    struct task_struct *next_task;
    if (ptable->proc_cnt > TASK_CNT) {
        perror("max task exceed");
        exit(EXIT_FAILURE);
    }

    next_task = &(ptable->proc[ptable->proc_cnt]);
    strcpy(next_task->cmd, cmd);
    
    next_task->priority = PRIO_DEFAULT;
    next_task->state = TASK_RUNNABLE;
    next_task->cpu_spent = 0;
    next_task->tot_mem = 0;
    next_task->pid = pid;
    ptable->proc_cnt++;
    sem_post(ptable_sem);
}

void os_core(struct proc_args *args) {
    INPUT = args->arg1; 
    input_buf = INPUT->data;
    input_sem = INPUT->lock.sem;

    PTABLE = args->arg2;
    ptable = PTABLE->data;
    ptable_sem = PTABLE->lock.sem;
    // Initialize lock for os_stat
    sem_post(ptable_sem);

    signal(SIGWINCH, resize_panes);
    printf("Welecome to Mini OS\n");

    while (1) {
        sem_wait(input_sem);
        printf("Received input: %s\n", input_buf);
        spawn_proc(os_spawn, register_proc, (void *)input_buf);
        resize_panes();
    }
}