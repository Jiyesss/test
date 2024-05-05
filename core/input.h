#include "helper.h"

// READLINE
#include <readline/readline.h>
#include <readline/history.h>

void handle_sigint(int sig) {
    printf("\nCtrl-C pressed. Press 'Ctrl-D' to quit.\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void os_input(struct shm_info *INPUT) {
    sem_t *input_sem = INPUT->lock.sem;
    char *INPUT_BUF = INPUT->data;
    char *input, *token;

    signal(SIGINT, handle_sigint);

    while (1) {
        input = readline("Enter command('exit' to quit) : ");
        if (input == NULL) {
            printf("Ctrl-D pressed. Exiting.\n");
            break;  // Ctrl-D가 눌리면 루프 종료
        }
        if (is_invalid(input))
            continue;

        // 빈 입력이 아닐 경우에만
        if (*input)
            add_history(input);

        strcpy(INPUT_BUF, input);
        sem_post(input_sem);
    }
}