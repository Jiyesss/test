#ifndef __UI_H
#define __UI_H

#include "constant.h"
#include "helper.h"

#define PANE_INPUT_Y 12
#define PANE_STATUS_Y 5

int count_panes() { //세션에 있는 tmux 패널의 수를 반환
    int cnt;

    shell("tmux list-panes -t MiniOS:0 | wc -l", "%d", &cnt);
    return cnt;
}

void reptyr_proc(pid_t pid) { //특정 프로세스를 tmux패널로 이동시키는 기능 수행
    int pos, count = count_panes();
    if (count < 3)
        pos = 0;
    else if (count == 3)
        pos = 2;
    else
        pos = count - 1;
    system_d("tmux split-window -t MiniOS:0.%d 'reptyr %d; sleep 1;'", pos, pid);
    system_d("tmux select-pane -t MiniOS:0.%d -T %d", pos+1, pid);
}

void resize_panes() {
    int window_height, pane_height;
    int count = count_panes() - 3;

    shell("tmux display-message -p '#{window_height}'", "%d", &window_height);
    system_d("tmux resize-pane -t MiniOS:0.0 -y %d", PANE_INPUT_Y);
    system_d("tmux resize-pane -t MiniOS:0.2 -y %d", PANE_STATUS_Y);

    if (count > 0) {
        // Calculate the target height for each pane
        pane_height = ((window_height - PANE_STATUS_Y) / count) - 1;

        // Resize each pane to the calculated height (except last)
        for (int i = 1; i < count; i++)
            system_d("tmux resize-pane -t MiniOS:0.%d -y %d", PANE_RIGHT + i, pane_height);
    }

    system_d("tmux select-pane -t MiniOS:0.0");
}


#endif