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
#ifdef CHANGED

#include "syscall_handlers.h"

typedef bool (*handler_ptr)(void);

#define SYSCALL_PROC(name, handler, ...)  handler,
#define SYSCALL_FUNC(name, handler, type, ...)  handler,

/*
 * here we create a table of function pointers to handlers for each
 * system call, this is actually really fast.
 *
 * the only issue of this implementation is the branch prediction miss
 * but this is negligible in our case.
 */
handler_ptr syscall_handlers[SYSCALL_COUNT] =
{
#include "syscall.def.h"
};

#undef SYSCALL_PROC
#undef SYSCALL_PROC

#endif

void
ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);
#ifdef CHANGED
    bool shall_update_pc;
#endif

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
	if (which == SyscallException) {

		if(type < 0 || type >= SYSCALL_COUNT)
		{
			printf("Invalid system call #%i\n", type);
			ASSERT(FALSE);
		}
		else
		{
			handler_ptr handler = syscall_handlers[type];
			if (handler != NULL)
			{
				IntStatus oldlevel = interrupt->SetLevel(IntOff);
				shall_update_pc = handler();
				interrupt->SetLevel(oldlevel);
			}
			else
			{
				printf("No handler for syscall %i\n", type);
				ASSERT(FALSE);
			}
		}

	}
	else if (which == ReadOnlyException)
	{
		unsigned guilty_vaddr = machine->ReadRegister(BadVAddrReg);

		if (currentThread->space->HandleReadOnly(guilty_vaddr))
		{
			DEBUG('a', "Read only exception at a CopyOnWrite location");
		}
		else
		{
			// JD: For now we do not handle this case, we simply
			// crash the system, a better solution would be:
			// If we have a signal system, send a SIGSEGV
			// If we don't, kill the process and don't stop the system
			printf("Write not allowed at virtual adress %i\n", guilty_vaddr);
			ASSERT(false);
		}

		// Now we will retry to execute the instruction
		shall_update_pc = false;
	}
	else
	{
		printf ("Unexpected user mode exception %d %d\n", which, type);
		ASSERT (FALSE);
	}


#endif // CHANGED

#ifdef CHANGED
    if(shall_update_pc)
      UpdatePC ();
#else
    // LB: Do not forget to increment the pc before returning!
    UpdatePC();
    // End of addition
#endif
}
