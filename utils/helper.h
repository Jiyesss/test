#ifndef __HELPER_H
#define __HELPER_H

#include "constant.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/ioctl.h>


struct proc_args {
    void *arg1;
    void *arg2;
    void *arg3;
};

struct sem_info {
    char *name;
    sem_t *sem;
};

struct shm_info {
    struct sem_info lock;
    char *name;
    void *data;
    size_t sz;
    int fd;
};


// Return the number of columns (terminal width)
int get_cols() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return -1;
    }
    return ws.ws_col;
}

// 입력 문자열에 공백이 아닌 문자가 있는지 확인
// 공백이 아닌 문자를 찾으면 false 반환
// 문자열 전체가 공백인 경우 true 반환
int is_invalid(char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void system_d(char *fmt, ...) {
    char cmd[BUF_SZ];
    va_list args;

    va_start(args, fmt);
    vsnprintf(cmd, sizeof(cmd), fmt, args);
    va_end(args);

    // printf("%s\n", cmd);
    system(cmd);
}

void shell(char *cmd, char *fmt, void *ptr) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command\n");
        exit(1);
    }
    
    fscanf(fp, fmt, ptr);
    pclose(fp);
}

char *generate_random_string(const char *prefix) {
    char template[1024] = "/tmp/";

    // PREFIX와 임시 파일 이름을 합침
    if (prefix) {
        strcat(template, prefix);
        strcat(template, ".");
    }
    strcat(template, "XXXXXX");

    // 임시 파일 이름 생성 (파일은 생성되지 않음)
    if (mktemp(template) == NULL) {
        perror("mktemp");
        exit(EXIT_FAILURE);
    }

    // 랜덤 문자열 부분만 반환
    // 메모리 누수를 방지하기 위해 strdup로 문자열 복사 후 반환
    return strdup(&template[5]);
}

// 세마포어 생성 함수
struct sem_info create_sem(const char *prefix) {
    struct sem_info semaphore;
    semaphore.name = generate_random_string(prefix);
    semaphore.sem = sem_open(semaphore.name, O_CREAT | O_EXCL, 0666, 0);
    if (semaphore.sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return semaphore;
}

// 세마포어 제거 함수
void destroy_sem(struct sem_info *semaphore) {
    if (sem_close(semaphore->sem) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(semaphore->name) == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    free(semaphore->name);
}

struct shm_info create_shm(const char *prefix, size_t size) {
    struct shm_info shmem;
    shmem.name = generate_random_string(prefix);
    shmem.fd = shm_open(shmem.name, O_CREAT | O_RDWR, 0666);
    if (shmem.fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmem.fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    shmem.sz = size;
    shmem.data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmem.fd, 0);
    if (shmem.data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    shmem.lock = create_sem(SEM_NAME);
    memset(shmem.data, 0, size);
    return shmem;
}

void destroy_shm(struct shm_info *shmem) {
    if (munmap(shmem->data, shmem->sz) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (close(shmem->fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(shmem->name) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    destroy_sem(&(shmem->lock));
    free(shmem->name);
}


#endif