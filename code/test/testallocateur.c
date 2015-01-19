#include "syscall.h"
#include "malloc_stub.h"
int main() {
    PutString("trying malloc...\n");
    int *a =(int*) malloc( sizeof(int));
    PutString("is it right ?");
    a[0] = 42;
    PutInt(a[0]);
    PutString("\ntrying free...\n");
    free(a);
    return 0;
}
