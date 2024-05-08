#ifndef __CONSTANT_H
#define __CONSTANT_H

#define SEM_NAME  "/sem"
#define SHM_ORDER "/order"
#define ORDER_PARENT 1
#define ORDER_CHILD 0

#define PANE_LEFT 0
#define PANE_RIGHT 2

#define BUF_SZ 256
#define TASK_CNT 32

#define PRIO_DEFAULT            20

#define TASK_RUNNABLE           0
#define TASK_RUNNING            1
#define TASK_BLOCKED            2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8

const char *task_status_str[] = { 
    [TASK_RUNNABLE] = "RUNNABLE",
    [TASK_RUNNING] = "RUNNING",
    [TASK_BLOCKED] = "BLOCKED",
    [TASK_ZOMBIE] = "ZOMBIE",
    [TASK_STOPPED] = "STOPPED"
};

struct task_struct {
    char cmd[BUF_SZ];
    pid_t pid;
    int state;
    int tot_mem;
    int cpu_spent;
    int priority;
};

struct proc_table {
    int proc_cnt;
    struct task_struct proc[TASK_CNT];
};

#define PTABLE_SZ sizeof(struct proc_table)

void iterate_ptable(struct proc_table *ptable) {
    if (ptable->proc_cnt > 0) {
        for (int i = 0; i < ptable->proc_cnt; i++) {
            struct task_struct *p = &(ptable->proc[i]);
            printf("PID: %d, %s\n", p->pid, task_status_str[p->state]);
        }
    }
}

#endif