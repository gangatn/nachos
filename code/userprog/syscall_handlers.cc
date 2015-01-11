#ifndef SYSCALL_HANDLERS_H_
#define SYSCALL_HANDLERS_H_

#include "system.h"
#include "userthread.h"

/*
 * Utility macro to get params and return value
 * --------------------------------------------
 *
 * PARAM(n):
 * ~~~~~~~~~
 * return the nth parameter, parameters starts from 1
 *
 * RETURN(val):
 * ~~~~~~~~~~
 * return the given value
 */

#define PARAM(n) machine->ReadRegister((3+n))
#define RETURN(val) machine->WriteRegister(2, (val))

void syscall_halt(void)
{

	DEBUG ('a', "Shutdown, initiated by user program.\n");
	interrupt->Halt ();
}

void do_syscall_exit(int s)
{
  DEBUG('a', "exiting with code %d\n", s);
  interrupt->Halt();
}

void syscall_exit(void)
{
  int ret = PARAM(1);
  do_syscall_exit(ret);
}

void syscall_putchar(void)
{
	char ch = PARAM(1);
	synchconsole->SynchPutChar(ch);
}


void copyStringFromMachine(int from, char *to, unsigned int size)
{
  unsigned int i = 0;
  int ch;

  while (i < size-1) {
    ASSERT(machine->ReadMem(from+i, 1, &ch));
    to[i] = (char)ch;
    ++i;
    if (ch == '\0')
      return;
  }

  to[size-1] = '\0';
}

void syscall_putstring(void)
{
	int mipsptr = PARAM(1);
	char str[MAX_STRING_SIZE];

	copyStringFromMachine(mipsptr, str, MAX_STRING_SIZE);
	synchconsole->SynchPutString(str);
}

void syscall_getchar(void)
{
	int ch = synchconsole->SynchGetChar();
	RETURN(ch);
}

void syscall_getstring(void)
{
	int i;
	int addr = PARAM(1);
	int size = PARAM(2);
	int ch;

	ASSERT(size >= 0);

	for (i = 0; i < size - 1; i++) {
		ch = synchconsole->SynchGetChar();

		if(ch == EOF)
			break;

		ASSERT(machine->WriteMem(addr+i, 1, ch));

		if (ch == '\n' || ch == '\0') {
			i++;
			break;
		}
	}

	ASSERT(machine->WriteMem(addr+i, 1, '\0'));
}

void syscall_putint(void)
{
	int val = PARAM(1);
	synchconsole->SynchPutInt(val);
}

void syscall_getint(void)
{
	int val;
	int addr = PARAM(1);
	synchconsole->SynchGetInt(&val);
	machine->WriteMem(addr, sizeof(val), val);
}

void syscall_userthreadcreate(void)
{
  int funcptr = PARAM(1);
  int argptr = PARAM(2);

  int ret = do_UserThreadCreate(funcptr, argptr);
  RETURN(ret);
}

void syscall_userthreadexit(void)
{
  do_UserThreadExit();
}

void syscall_userthreadjoin(void)
{
  int tid = PARAM(1);
  do_UserThreadJoin(tid);
}


#endif /* SYSCALL_HANDLERS_H_ */
