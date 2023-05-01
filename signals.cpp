#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    printf("smash: got ctrl-Z\n");
    SmallShell &smashy = SmallShell::getInstance();
    int worked = 0;
    if (smashy.getRunPid() != smashy.getPid()) {
        worked = kill(smashy.getRunPid(), SIGSTOP);
        if (worked == -1) {
            std::perror("smash: error: kill failed");
        }
        cout << "smash: process:" << smashy.getRunPid() << " was stopped" << endl;
        smashy.getJobsList()->addJob(smashy.getRunPid(),smashy.getCmdLine(),smashy.getCurrNumArgs(),true);
        smashy.nullifyCmdStats();
    }
    smashy.setRunningPid(-1);
}

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    printf("smash: got ctrl-C\n");
    SmallShell &smashy = SmallShell::getInstance();
    int worked = 0;
    if (smashy.getRunPid() != smashy.getPid()) {
        worked = kill(smashy.getRunPid(), SIGKILL);
        if (worked == -1) {
            std::perror("smash: error: kill failed");
        }
        cout << "smash: process:" << smashy.getRunPid() << " was killed" << endl;
    }
    smashy.setRunningPid(-1);
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}

