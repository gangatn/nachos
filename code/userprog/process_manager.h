// Process manager
//
// System use a process manager (minix like)
// to manipulate processes

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

class ProcessManager
{
public:
	ProcessManager();

	~ProcessManager();

	void Init(const char *filename, int pid, int ppid);

	int Fork();

	int Exit(int exit_code);

	int WaitPid(int pid);

	int Exec(const char *filename);


private:
	void initProcess(Thread *thread);
};

#endif /* PROCESS_MANAGER_H */
