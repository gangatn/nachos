#include "syscall.h"

int main(void)
{
  int pid = 1;

  PutString("Testing process fork\n");

  pid = Fork();

  if (pid == 0)
  {
    PutString("I am the child\n");
  }
  else {
    PutString("I am the father\n");
  }

  PutString("ok thx bye\n");

  return 0;
}
