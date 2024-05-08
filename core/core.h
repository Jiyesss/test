#include "constant.h"
#include "helper.h"
#include "ui.h"

static struct shm_info *INPUT;
static char *input_buf;
static sem_t *input_sem;

static struct shm_info *PTABLE; //입력 버퍼와 프로세스 테이블을 관리하기 위한 공유 메모리 구조체 포인터
static struct proc_table *ptable;
static sem_t *ptable_sem;

void isolate_proc() { // 현재 프로세스를 새로운 프로세스 그룹에 할당하여, 이 프로세스가 독립적으로 작동할 수 있도록 함
    pid_t pid = getpid();
    if (setpgid(pid, pid) == -1) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
}

// Assure parent start after child signals
void spawn_proc(void (*child)(), void (*parent)(), void *arg) { // 자식 프로세스를 생성하고, 자식과 부모 프로세스에 각각 다른 함수를 할당함, 공유 메모리를 사용하여 순서를 제어하며, 자식프로세스 먼저 실행
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

void parse_cmd(char *cmd, char **args) { // 명령어 스트링을 파싱하여, 인자 배열로 변환, 따옴표로 둘러싸인 인자를 처리할 수 있으며, 공백 기준으로 인자 구분
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

void os_spawn(char *cmd) { // 파싱된 명령어를 바탕으로 새 프로세스를 생성하고, 해당 프로세스에서 명령어를 실행, exe cvp함수를 사용하여, 현재 프로세스 이미지를 새로운 프로그램 이미지로 대체함
    char *args[BUF_SZ];
    int ret;

    parse_cmd(cmd, args);
    ret = execvp(args[0], args);
    if (ret != 0) {
        printf("'%s' %s\n", cmd, strerror(errno));
    }
}

void register_proc(pid_t pid, char *cmd) { // 생성된 프로세스를 프로세스 테이블에 등록, 세마포어를 사용하여 테이블 접근을 동기화하고, 프로세스 정보를 저장함
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

void os_core(struct proc_args *args) { // 모의 운영 체제의 핵심 루프를 실행함. 사용자 입력을 받아 해당 명령어를 실행하고, 실행된 프로세스를 관리
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