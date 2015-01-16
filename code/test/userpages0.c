#include "syscall.h"

void writeChars(void * arg) {
    int n = (int) arg;
    char a = n % 2 == 0  ? 'a' : 'A';
    char i;
    for (i = a ; i < a + 26; ++i) {
        PutChar(i);
    }
}

int main() {
    int t1 = UserThreadCreate(writeChars, (void*)1);
    int t2 = UserThreadCreate(writeChars, (void*)2);
    UserThreadJoin(t1);
    UserThreadJoin(t2);
    return 0;
}
