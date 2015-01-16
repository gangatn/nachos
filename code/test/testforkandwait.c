#include "syscall.h"

int main(void)
{
  int pid;
  int i;

  pid = Fork();

  if (pid != 0) {
    PutString("I am the father, waiting for the son\n");
    WaitPid(pid);
    PutString("Son terminated\n");
  } else {
    PutString("I am the son\n");
    for (i = 0 ; i < 100000 ; ++i);
    PutString("Son exit now\n");
    Exit(0);
  }
  
  return 0;
}
