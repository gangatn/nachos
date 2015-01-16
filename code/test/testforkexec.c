#include "syscall.h"

int main(void) {
    int pid;

    pid = ForkExec("putstring");

    if (pid != 0)
        PutString("I am the father\n");

    return 0;
}
