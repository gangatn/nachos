#include "userthread.h"
#include "system.h"

static void StartUserThread(int f)
{
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();

  /* Set PC to f */
  machine->WriteRegister(PCReg, f);
  machine->WriteRegister(NextPCReg, f+4);
  /* Pass the arg to f*/
  machine->WriteRegister(4, currentThread->userarg);
  /* SP */
  machine->WriteRegister(StackReg, (currentThread->space->getNumPages() - 3 * currentThread->tid ) * PageSize - 16);

  machine->Run();  
}

int do_UserThreadCreate(int f, int arg)
{
  /* update the number of threads in the process */
  *(currentThread->threadcount) += 1;

  Thread *newThread = new Thread("userthread");

  /* pass the arg to the new thread */
  newThread->userarg = arg;

  /* give the thread count to the new thread
   * and set the tid for the new thread
   */
  newThread->threadcount = currentThread->threadcount;
  newThread->tid = *(newThread->threadcount);


  /* init the joining semaphore of the new thread */
  newThread->joinsem = new Semaphore("semjoin", 0);
  
  if (!currentThread->children)
    currentThread->children = new List();

  /* add the new thread in the thread list of the current thread */
  currentThread->children->Append((void*)newThread);
  
  newThread->Fork(StartUserThread, f);

  return newThread->tid;
}

void do_UserThreadExit()
{
  currentThread->ReapChildren();
  currentThread->joinsem->V();
  currentThread->Sleep();

  /* never reached */
  ASSERT(FALSE);
}
