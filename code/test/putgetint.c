#include "syscall.h"

#define MAGIC_NUMBER 233

int main()
{
  int n;

  PutString("What is the magic number?: ");
  GetInt(&n);

  if (n == MAGIC_NUMBER) {
    PutString("Correct, ");
    PutInt(n);
    PutString(" is the magic number!\n");
  } else {
    PutString("Nope, ");
    PutInt(n);
    PutString(" is not the magic number.\n");
  }

  return 0;
}
