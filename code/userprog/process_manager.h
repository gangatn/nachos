// Process manager
//
// System use a process manager (minix like)
// to manipulate processes

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "thread.h"
#include "bitmap.h"
#include <list>

#define MAX_PROCESS 65536

struct process_entry;

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

	int getppid(int pid);
private:
	BitMap pids;
	void initProcess(Thread *thread, int pid, int ppid);

	std::list<struct process_entry*> processlist;
};

#endif /* PROCESS_MANAGER_H */
