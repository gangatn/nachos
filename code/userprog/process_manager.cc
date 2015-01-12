#include "process_manager.h"
#include "system.h"
#include "addrspace.h"
#include <list>
#include <queue>

ProcessManager::ProcessManager()
{

}

ProcessManager::~ProcessManager()
{

}

void ProcessManager::initProcess(Thread *th, int pid, int ppid)
{
	th->maxtid = new int;
	*(th->maxtid) = 1;

	th->tid = 0;

	th->userThreads = new list<Thread*>;
	th->userThreads->push_back(th);

	th->avaiabletids = new queue<int>;

	th->threadcount = new int;

	*(th->threadcount) = 1;

}

void ProcessManager::Init(const char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		ASSERT(FALSE && "Can't open init file, aborting\n");
	}

	space = new AddrSpace(executable);
	currentThread->space = space;

	initProcess(currentThread);

	delete executable;

	space->InitRegisters();
	space->RestoreState();

	machine->Run ();
	ASSERT (FALSE); 			// Shall never be reached
}

static void DoFork(int arg)
{
	// TODO:
	//
	// WE NEED TO RETURN THE VALUE 0
	// AND ADVANCE PC
	//
	// SAME FOR LR
	// SP SHOULD BE RESTORED
	machine->Run();
}

int ProcessManager::Fork()
{
	AddrSpace *space = new AddrSpace(currentThread->space);
	Thread *newThread = new Thread("user process");
	int pid; // ALLOCATE PID

	newThread->space = space;

	initProcess(newThread);

	newThread->Fork(DoFork, 0);
}

int ProcessManager::Exec(const char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		ASSERT(FALSE && "Can't open init file, aborting\n");
	}

	space = new AddrSpace(executable);

	delete currentThread->space;

	currentThread->space;

	space->InitRegisters();
}

int ProcessManager::Exit(int exit_code)
{

}

int ProcessManager::WaitPid(int pid)
{

}
