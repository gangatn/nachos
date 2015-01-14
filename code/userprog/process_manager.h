// Process manager
//
// System use a process manager (minix like)
// to manipulate processes

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "thread.h"
#include "bitmap.h"

#define MAX_PROCESS 65536

class ProcessManager
{
public:
	ProcessManager();

	~ProcessManager();

	void Init(char *filename);

	int Fork();

	void Exit(int exit_code);

	int WaitPid(int pid);

	int Exec(char *filename);

	int ForkExec(char *filename);
private:
	BitMap pids;
	void initProcess(Thread *thread, int pid, int ppid);
};

#endif /* PROCESS_MANAGER_H */
