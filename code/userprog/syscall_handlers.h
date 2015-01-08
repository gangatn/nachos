#ifndef SYSCALL_HANDLERS_H_
#define SYSCALL_HANDLERS_H_

#include "system.h"

static void syscall_halt(void)
{

	DEBUG ('a', "Shutdown, initiated by user program.\n");
	interrupt->Halt ();
}

static void syscall_exit(void)
{
	int ret = machine->ReadRegister(4);
	DEBUG('a', "exiting with code %d\n", ret);
	interrupt->Halt();
}

static void syscall_putchar(void)
{
	char ch = machine->ReadRegister(4);
	synchconsole->SynchPutChar(ch);
}


static void copyStringFromMachine(int from, char *to, unsigned int size)
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

static void syscall_putstring(void)
{
	int mipsptr = machine->ReadRegister(4);
	char str[MAX_STRING_SIZE];

	copyStringFromMachine(mipsptr, str, MAX_STRING_SIZE);
	synchconsole->SynchPutString(str);
}

static void syscall_getchar(void)
{
	int ch = synchconsole->SynchGetChar();
	machine->WriteRegister(2, ch);
}

static void syscall_getstring(void)
{
	int i;
	int addr = machine->ReadRegister(4);
	int size = machine->ReadRegister(5);
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

static void syscall_putint(void)
{
	int val = machine->ReadRegister(4);
	synchconsole->SynchPutInt(val);
}

static void syscall_getint(void)
{
	int val;
	int addr = machine->ReadRegister(4);
	synchconsole->SynchGetInt(&val);
	machine->WriteMem(addr, sizeof(val), val);
}


#endif /* SYSCALL_HANDLERS_H_ */
