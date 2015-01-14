#include "process_manager.h"
#include "system.h"
#include "addrspace.h"
#include <list>
#include <queue>

using namespace std;

ProcessManager::ProcessManager()
	: pids(MAX_PROCESS)
{
	pids.Mark(0);
}

ProcessManager::~ProcessManager()
{

}

void ProcessManager::initProcess(Thread *th, int pid, int ppid)
{
	th->maxtid = new int;
	*(th->maxtid) = 1;

	th->tid = 0;

	th->userthreads = new list<Thread *>;
	th->userthreads->push_back(th);

	th->availabletids = new queue<int>;

	th->threadcount = new int;

	*(th->threadcount) = 1;

	th->pid = pid;
	th->ppid = ppid;

}

void ProcessManager::Init(char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		ASSERT(FALSE && "Can't open init file, aborting\n");
	}

	space = new AddrSpace(executable);
	currentThread->space = space;
	pids.Mark(1);
	initProcess(currentThread, 1, 0);

	delete executable;

	space->InitRegisters();
	space->RestoreState();

	machine->Run ();
	ASSERT (FALSE); 			// Shall never be reached
}

static void DoFork(int arg)
{
	machine->Run();
}

int ProcessManager::Fork()
{
	AddrSpace *space = new AddrSpace(*currentThread->space);
	Thread *newThread = new Thread("user process");
	int pid; // ALLOCATE PID
	unsigned int i;

	pid = pids.Find();

	newThread->space = space;

	initProcess(newThread, pid, currentThread->pid);
	for(i = 0; i < NumTotalRegs; i++)
	{
		newThread->userRegisters[i] = machine->ReadRegister(i);
	}

	newThread->userRegisters[PCReg] = newThread->userRegisters[NextPCReg];
	newThread->userRegisters[NextPCReg] += 4;
	newThread->userRegisters[2] = 0; // We set the return value of zero

	newThread->Fork(DoFork, 0);

	return pid;
}

int ProcessManager::Exec(char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		puts("ERROR EXEC");
		return -1;
	}

	space = new AddrSpace(executable);

	delete currentThread->space;

	currentThread->space = space;

	space->InitRegisters();
	space->RestoreState();
	return 0;
}

void ProcessManager::Exit(int exit_code)
{
	pids.Clear(currentThread->pid);

	if(pids.NumClear() >= MAX_PROCESS - 1)
	{
		interrupt->Halt();
	}
	else
	{
		currentThread->Finish();
	}
}

int ProcessManager::WaitPid(int pid)
{
	return -1;
}
