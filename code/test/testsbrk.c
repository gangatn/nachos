#include "syscall.h"

int main() {

    PutString("Sbrk(0) = ");
    void *brk = Sbrk(262144);

    PutInt((int)Sbrk(0));
    PutChar('\n');
    brk = ((char*)brk) + 400;

    if (Sbrk(200) == (void *) - 1)
        PutString("Error \n");
    int *test = (int*)brk;

    *test = 42;

    PutInt(*test);
    PutChar('\n');
    return 0;
}
