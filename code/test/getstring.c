#include "syscall.h"

#define BUFFER_SIZE 42

int main() {
    char buffer[BUFFER_SIZE];

    PutString("What is your name ? : ");
    GetString(buffer, BUFFER_SIZE);
    PutString("Yay, your name is ");
    PutString(buffer);

    return 0;
}
