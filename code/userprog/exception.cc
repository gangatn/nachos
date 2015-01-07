// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}


#ifdef CHANGED
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
#endif // CHANGED

//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);

#ifdef CHANGED
    char ch;
    char str[MAX_STRING_SIZE];
    int mipsptr;
    int ret;
    int addr;
    int size;
    int i;
#endif // CHANGED

#ifndef CHANGED
    if ((which == SyscallException) && (type == SC_Halt))
      {
	  DEBUG ('a', "Shutdown, initiated by user program.\n");
	  interrupt->Halt ();
      }
    else
      {
	  printf ("Unexpected user mode exception %d %d\n", which, type);
	  ASSERT (FALSE);
      }
#else // CHANGED
    if (which == SyscallException)
      {
	switch(type)
	  {
	  case SC_Halt:
	    DEBUG ('a', "Shutdown, initiated by user program.\n");
	    interrupt->Halt ();
	    break;
	    
	  case SC_Exit:
	    ret = machine->ReadRegister(4);
	    DEBUG('a', "Ret %d\n", ret);
	    interrupt->Halt();
	    break;

	  case SC_PutChar:
	    ch = (char)machine->ReadRegister(4);
	    synchconsole->SynchPutChar(ch);
	    break;
	    
	  case SC_PutString:
	    mipsptr = machine->ReadRegister(4);
	    copyStringFromMachine(mipsptr, str, MAX_STRING_SIZE);
	    synchconsole->SynchPutString(str);
	    break;

	  case SC_GetChar:
	    ch = synchconsole->SynchGetChar();
	    machine->WriteRegister(2, (int)ch);
	    break;

	  case SC_GetString:
	    addr = machine->ReadRegister(4);
	    size = machine->ReadRegister(5);
	    ASSERT(size >= 0);

	    for (i = 0 ; i < size-1 ; ++i) {
	      ch = synchconsole->SynchGetChar();
	      if (ch == EOF)
		break; 
	      ASSERT(machine->WriteMem(addr+i, 1, (int)ch));
	      if (ch == '\n' || ch == '\0') {
		++i;
		break;
	      }
	    }
	    ASSERT(machine->WriteMem(addr+i, 1, '\0'));
	    break;

	  case SC_PutInt:
	    i = machine->ReadRegister(4);
	    synchconsole->SynchPutInt(i);
	    break;

	  case SC_GetInt:
	    addr = machine->ReadRegister(4);
	    synchconsole->SynchGetInt(&i);
	    machine->WriteMem(addr, sizeof(int), i);
	    break;
	    
	  default:
	    printf ("Unexpected user mode exception %d %d\n", which, type);
	    ASSERT (FALSE);
	  }
      }
#endif // CHANGED

    // LB: Do not forget to increment the pc before returning!
    UpdatePC ();
    // End of addition
}
