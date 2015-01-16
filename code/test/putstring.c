#include "syscall.h"

void merlin_says(char *s) {
    PutString("Merlin sings: ");
    PutString(s);
    PutChar('\n');
}

int main() {
    merlin_says("Hockety Pockety Wockety Wack...");
    return 0;
}
