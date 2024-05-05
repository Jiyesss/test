#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMANDS 40
#define BUFFER_SIZE 256

// 명령 구조체
struct tmux_cmdr {
    char *cmds[MAX_COMMANDS];
    int cnt;
};

struct tmux_cmdr tmux_init() {
    struct tmux_cmdr cmdr;
    cmdr.cnt = 0;
    return cmdr;
}

void tmux_clear(struct tmux_cmdr *cmdr) {
    for (int i = 0; i < cmdr->cnt; i++) {
        free(cmdr->cmds[i]);
    }
    cmdr->cnt = 0;
}

void tmux_add(struct tmux_cmdr *cmdr, char *command) {
    if (cmdr->cnt < MAX_COMMANDS) {
        cmdr->cmds[cmdr->cnt++] = strdup(command);
    } else {
        fprintf(stderr, "Command limit reached.\n");
    }
}

void tmux_run(struct tmux_cmdr *cmdr) {
    char buffer[BUFFER_SIZE] = "tmux ";
    int length = strlen(buffer);

    for (int i = 0; i < cmdr->cnt; i++) {
        length += snprintf(buffer + length, BUFFER_SIZE - length, "%s ", cmdr->cmds[i]);
        if (i < cmdr->cnt - 1) {
            length += snprintf(buffer + length, BUFFER_SIZE - length, "\\; ");
        }
    }

    printf("Executing command: %s\n", buffer);
    system(buffer);

    tmux_clear(cmdr);
}

// int main() {
//     struct tmux_cmdr tmux = tmux_init();
//     tmux_add(&tmux, "split-window -h");
//     tmux_add(&tmux, "split-pane -h");
//     tmux_add(&tmux, "select-window -h");
//     tmux_add(&tmux, "select-pane -h");
//     tmux_run(&tmux);

//     tmux_add(&tmux, "split-window -h");
//     tmux_add(&tmux, "select-window -h");
//     tmux_add(&tmux, "select-pane -h");

//     tmux_run(&tmux);
// }
