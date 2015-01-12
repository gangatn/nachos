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

	void Init(const char *filename);

	int Fork();

	int Exec(const char *filename);


private:
};

#endif /* PROCESS_MANAGER_H */
