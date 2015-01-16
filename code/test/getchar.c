#include "syscall.h"

int main() {
    char c = GetChar();
    PutChar(c);

    return 0;
}
