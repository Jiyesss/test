#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>


// 시스템 상태 실시간 모니터링 및 표시
static struct proc_table *ptable;
static sem_t *ptable_sem;

double calculate_cpu_usage() {
    long double a[4], b[4], loadavg;
    FILE *fp;
    static long double prev[4] = {0.0, 0.0, 0.0, 0.0}; // 이전 값 저장을 위한 정적 변수

    fp = fopen("/proc/stat","r");
    if (fp == NULL) {
        perror("Error opening file");
        return -1; // 오류 처리
    }
    if (fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]) < 4) {
        perror("Error reading file");
        fclose(fp);
        return -1; // 오류 처리
    }
    fclose(fp);

    // 첫 번째 호출에서는 이전 값(prev)이 없으므로 현재 값을 prev에 저장하고 -1을 반환
    if (prev[0] == 0 && prev[1] == 0 && prev[2] == 0 && prev[3] == 0) {
        prev[0] = a[0];
        prev[1] = a[1];
        prev[2] = a[2];
        prev[3] = a[3];
        return -1;
    }

    // CPU 사용량 계산
    long double total_prev = prev[0] + prev[1] + prev[2] + prev[3];
    long double total_now = a[0] + a[1] + a[2] + a[3];
    long double total_diff = total_now - total_prev;
    long double idle_diff = a[3] - prev[3];

    loadavg = (total_diff - idle_diff) / total_diff;

    // 현재 값을 prev 배열에 저장
    prev[0] = a[0];
    prev[1] = a[1];
    prev[2] = a[2];
    prev[3] = a[3];

    return loadavg * 100;
}

double calculate_memory_usage() {
    long long total_memory = 0;
    long long free_memory = 0;
    char line[256];
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return -1; // 오류 처리
    }
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lld kB", &total_memory);
        }
        if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %lld kB", &free_memory);
            break; // 필요한 정보를 모두 읽었다면 루프를 탈출
        }
    }
    fclose(fp);
    if (total_memory == 0) {
        return -1; // 오류 처리
    }

    double used_memory = (double)(total_memory - free_memory) / total_memory;
    return used_memory * 100; // 백분율로 반환
}


void os_status(struct shm_info *PTABLE) { // 시스템의 상태를 표시함
    ptable = PTABLE->data;
    ptable_sem = PTABLE->lock.sem;

    while (1) {
        system_d("clear");
        
        double cpu_usage = calculate_cpu_usage(); // 실제 CPU 사용량 계산
        printf("CPU : [");
        int bar_width = get_cols() - 15;
        int pos = bar_width * (cpu_usage / 100.0);
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) printf("#");
            else printf(" ");
        }
        printf("] %.2f%%\n", cpu_usage);
        printf("\n");
        

        // 메모리 사용량 표시
        double mem_usage = calculate_memory_usage(); // 실제 메모리 사용량 계산
        printf("MEM : [");
        int mem_bar_width = get_cols() - 15;
        int mem_pos = mem_bar_width * (mem_usage / 100.0);
        for (int i = 0; i < mem_bar_width; ++i) {
            if (i < mem_pos) printf("#");
            else printf(" ");
        }
        printf("] %.2f%%\n", mem_usage);

        sem_wait(ptable_sem);
        iterate_ptable(ptable);
        sem_post(ptable_sem);

        sleep(1); // 1초 대기
}
    }




