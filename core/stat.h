#include "helper.h"
// 시스템 상태 실시간 모니터링 및 표시
static struct proc_table *ptable;
static sem_t *ptable_sem;

void os_status(struct shm_info *PTABLE) { // 시스템의 상태를 표시함
    ptable = PTABLE->data;
    ptable_sem = PTABLE->lock.sem;

    while (1) {
        system_d("clear");
        printf("CPU : [");
        int bar_width = get_cols() - 15;
        double cpu_usage = 70; // 그냥 기본값인 70으로 계속 고정되어있음
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