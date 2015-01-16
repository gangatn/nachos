#ifdef CHANGED

#include "userthread.h"
#include "system.h"
#include "syscall_handlers.h"
#include <list>
#include <queue>

using namespace std;

static void StartUserThread(int f)
{
  currentThread->space->InitRegisters();

  /* Set PC to f */
  machine->WriteRegister(PCReg, f);
  machine->WriteRegister(NextPCReg, f+4);
  /* Set LR to Thread cleanup function
   * Statically located */
  machine->WriteRegister(RetAddrReg, 16);
  /* Pass the arg to f*/
  machine->WriteRegister(4, currentThread->userarg);
  /* SP */
  machine->WriteRegister(StackReg, (machine->pageTableSize - 3 * currentThread->tid ) * PageSize - 16);

  machine->Run();
}

int do_UserThreadCreate(int f, int arg)
{
  Thread *newThread = new Thread("userthread");

  /* pass the arg to the new thread */
  newThread->userarg = arg;

  /* give the available tids list to the new thread */
  newThread->availabletids = currentThread->availabletids;

  /* give the max tid to the new thread */
  newThread->maxtid = currentThread->maxtid;

  newThread->pid = currentThread->pid;

  /* set the tid of the new thread */
  if (newThread->availabletids->empty()) {
    newThread->tid = *(newThread->maxtid);
    *(currentThread->maxtid) += 1;
  } else {
    newThread->tid = newThread->availabletids->front();
    newThread->availabletids->pop();
  }

  newThread->threadcount = currentThread->threadcount;
  *(newThread->threadcount) += 1;

  /* add the new thread in the thread list of the current thread */
  currentThread->userthreads->push_back(newThread);

  /* give the thread list to the new thread */
  newThread->userthreads = currentThread->userthreads;

  newThread->space = currentThread->space;

  newThread->Fork(StartUserThread, f);

  return newThread->tid;
}

void do_UserThreadExit()
{
  if (*(currentThread->threadcount) == 1) {
    do_syscall_exit(0);
  }

  currentThread->joinsem->V();
  *(currentThread->threadcount) -= 1;
  currentThread->Sleep();

  /* never reached */
  ASSERT(FALSE);
}

void do_UserThreadJoin(int tid)
{
  list<Thread*> *userthreads = currentThread->userthreads;
  list<Thread*>::iterator it;
  Thread *t;

  /* tid must be different than the current userthread tid */
  if (tid == currentThread->tid)
    return;

  /* join the tid userthread */
  for (it = userthreads->begin() ; it != userthreads->end() ; ++it) {
    t = (*it);
    if (t->tid == tid) {
      t->joinsem->P(); // wait for the userthread to exit
      userthreads->erase(it); // remove the userthread from the list
      t->availabletids->push(t->tid); // make the tid available
      delete t; // deallocate the userthread
      return;
    }
  }

  /* tid not found */
}

#endif
