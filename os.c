#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "stat.h"
#include "core.h"

void init_tui() {
    struct shm_info INPUT = create_shm("INPUT", BUF_SZ);
    struct shm_info PTABLE = create_shm("PTABLE", PTABLE_SZ);
    struct proc_args core_args = {.arg1 = &INPUT,
                                  .arg2 = &PTABLE };

    system_d("tmux kill-session -t MiniOS 2>/dev/null");
    system_d("tmux new-session -d -s MiniOS");
    system_d("tmux set status-position top");
    system_d("tmux set mouse on");

    system_d("tmux split-window -h -t MiniOS:0.0");
    spawn_proc(os_input, NULL, &INPUT);
    system_d("tmux kill-pane -t MiniOS:0.0");
    spawn_proc(os_core, NULL, &core_args);
    
    spawn_proc(os_status, NULL, &PTABLE);
    system_d("tmux kill-pane -t MiniOS:0.2");
    resize_panes();
}

int main() {
    init_tui();

    system_d("echo 'Mini OS입니다.' | pv -qL 15");
    system_d("echo 'Booting up~' | pv -qL 15");
    system_d("echo '###################' | pv -qL 15");
    system_d("tmux attach");
    return 0;
}

