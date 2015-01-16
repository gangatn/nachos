#include "syscall.h"

void printpid() {
    PutString("pid=");
    PutInt(getpid());
    PutString(", ppid=");
    PutInt(getppid());
    PutString("\n");
}

int main(void) {
    int pid;
    int i;

    pid = Fork();

    if (pid != 0) {
        PutString("I am the father, fork and wait for the child | ");
        printpid();
        WaitPid(pid);
        PutString("Child terminated\n");
    } else {
        PutString("I am the child, fork and wait for the subchild | ");
        printpid();
        for (i = 0 ; i < 100000 ; ++i);
        pid = Fork();
        if (pid == 0) {
            PutString("I am the subchild | ");
            printpid();
            PutString("Subchild exit\n");
            Exit(0);
        }
        WaitPid(pid);
        PutString("Subchild terminated\n");
        Exit(0);
    }

    pid = Fork();

    if (pid == 0) {
        printpid();
        pid = Fork();
        if (pid != 0) {
            Exit(0);
        } else {
            for (i = 0 ; i < 100000 ; ++i);
            printpid();
            Exit(0);
        }
    } else
        WaitPid(pid);

    return 0;
}
