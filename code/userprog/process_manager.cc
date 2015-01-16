#include "process_manager.h"
#include "system.h"
#include "addrspace.h"
#include <list>
#include <queue>
#include "synch.h"

using namespace std;

ProcessManager::ProcessManager()
	: pids(MAX_PROCESS)
	, processlist()
{
	pids.Mark(0);
}

ProcessManager::~ProcessManager()
{
  list<struct process_entry*>::iterator it;
  struct process_entry *entry;
  
  it = processlist.begin();

  while(!processlist.empty()) {
    entry = *it;
    processlist.erase(it);
    delete entry->semproc; 
   delete entry;
  }
}

void ProcessManager::initProcess(Thread *th, int pid, int ppid)
{
	struct process_entry *entry = new struct process_entry;
	entry->semproc = new Semaphore("process semaphore", 0);

	th->maxtid = new int;
	*(th->maxtid) = 1;

	th->tid = 0;

	th->userthreads = new list<Thread *>;
	th->userthreads->push_back(th);

	th->availabletids = new queue<int>;

	th->threadcount = new int;

	*(th->threadcount) = 1;

	th->pid = pid;

	entry->ppid = ppid;
	entry->pid = pid;
	processlist.push_front(entry);
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
	AddrSpace *space = new AddrSpace(currentThread->space, true);
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
		DEBUG('t', "ERROR EXEC\n");
		return -1;
	}

	delete currentThread->space;
	space = new AddrSpace(executable);

	delete executable;

	currentThread->space = space;

	space->InitRegisters();
	space->RestoreState();

	return 0;
}

/* DoForkExec
 * Init the registers, restore the thread state in the machine
 * and runs the thread
 */
static void DoForkExec(int arg)
{
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();

  machine->Run();

  /* You *shall* never reach here! */
  ASSERT(FALSE);
}

/* ForkExec
 * Forks and exec the `filename` program
 * Returns the process pid in the father
 */
int ProcessManager::ForkExec(char *filename)
{
  int pid;
  Thread *newThread;
  OpenFile *executable;
  AddrSpace *space;

  pid = pids.Find();
  executable = fileSystem->Open(filename);
  space = new AddrSpace(executable);
  delete executable;
  newThread = new Thread("user process");
  newThread->space = space;

  initProcess(newThread, pid, currentThread->pid);
  newThread->Fork(DoForkExec, 0);

  return pid;
}

void ProcessManager::Exit(int exit_code)
{
  	list<struct process_entry*>::iterator it;
  	struct process_entry *entry;

	pids.Clear(currentThread->pid);

	if(pids.NumClear() >= MAX_PROCESS - 1)
	{
		interrupt->Halt();
	}
	else
	{
	  // Tell that the process has terminated to the father
	  for (it = processlist.begin() ; it != processlist.end() ; ++it) {
	    entry = *it;
	    if (entry->pid == currentThread->pid) {
	      entry->semproc->V();
	      break;
	    }
	  }

	  // Attach all processes which the ppid matches the pid of the currentThread to Init
	  for (it = processlist.begin() ; it != processlist.end() ; ++it) {
	    entry = *it;
	    if (entry->ppid == currentThread->pid) {
	      entry->ppid = 1;
	    }
	  }
	  
	  currentThread->Finish();
	}
}

int ProcessManager::WaitPid(int pid)
{
  list<struct process_entry*>::iterator it;
  struct process_entry *entry;

  for (it = processlist.begin() ; it != processlist.end() ; ++it) {
    entry = *it;
    if (entry->pid == pid) {
      // check that pid is the ppid child
      if (currentThread->pid != entry->ppid)
	return -1;
      entry->semproc->P();
      processlist.erase(it);
      pid = entry->pid;
      delete entry->semproc;
      delete entry;
      return pid;
    }
  }
  
  // PID not found
  return -1;
}

int ProcessManager::getppid(int pid)
{
  list<struct process_entry*>::iterator it;

  for (it = processlist.begin() ; it != processlist.end() ; ++it)
    if ((*it)->pid == pid)
      return (*it)->ppid;

  return -1;
}
