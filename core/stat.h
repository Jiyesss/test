#include "helper.h"

static struct proc_table *ptable;
static sem_t *ptable_sem;

void os_status(struct shm_info *PTABLE) {
    ptable = PTABLE->data;
    ptable_sem = PTABLE->lock.sem;

    while (1) {
        system_d("clear");
        printf("CPU : [");
        int bar_width = get_cols() - 15;
        double cpu_usage = 70;
        int pos = bar_width * (cpu_usage / 100.0);
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) printf("#");
            else printf(" ");
        }
        printf("] %.2f%%\n", cpu_usage);

        sem_wait(ptable_sem);
        iterate_ptable(ptable);
        sem_post(ptable_sem);

        sleep(1); // 1초 대기
    }
}