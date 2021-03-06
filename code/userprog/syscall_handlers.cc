#ifndef SYSCALL_HANDLERS_H_
#define SYSCALL_HANDLERS_H_

#include "system.h"
#include "userthread.h"
#include "openfile_table.h"

/*
 * Utility macro to get params and return value
 * --------------------------------------------
 *
 * PARAM(n):
 * ~~~~~~~~~
 * return the nth parameter, parameters starts from 1
 *
 * RETURN(val):
 * ~~~~~~~~~~~~
 * return the given value
 */

#define PARAM(n) machine->ReadRegister((3+n))
#define RETURN(val) machine->WriteRegister(2, (val))

bool syscall_halt(void) {

    DEBUG ('a', "Shutdown, initiated by user program.\n");
    interrupt->Halt ();
    return true;
}

bool do_syscall_exit(int s) {
    DEBUG('a', "exiting with code %d\n", s);
    processmanager->Exit(s);
    return true;
}

bool syscall_exit(void) {
    int ret = PARAM(1);
    do_syscall_exit(ret);
    return true;
}

bool syscall_putchar(void) {
    char ch = PARAM(1);
    synchconsole->SynchPutChar(ch);
    return true;
}


static void copyStringFromMachine(int from, char *to, unsigned int size) {
    unsigned int i = 0;
    int ch;

    while (i < size - 1) {
        ASSERT(machine->ReadMem(from + i, 1, &ch));
        to[i] = (char)ch;
        ++i;
        if (ch == '\0')
            return;
    }

    to[size - 1] = '\0';
}

static void copyBufToMachine(char *from, int to, unsigned size) {
    unsigned i;

    for (i = 0; i < size; i++)
        ASSERT(machine->WriteMem(to + i, 1, from[i]));
}

bool syscall_putstring(void) {
    int mipsptr = PARAM(1);
    char str[MAX_STRING_SIZE];

    copyStringFromMachine(mipsptr, str, MAX_STRING_SIZE);
    synchconsole->SynchPutString(str);
    return true;
}

bool syscall_getchar(void) {
    int ch = synchconsole->SynchGetChar();
    RETURN(ch);
    return true;
}

bool syscall_getstring(void) {
    int i;
    int addr = PARAM(1);
    int size = PARAM(2);
    int ch;

    ASSERT(size >= 0);

    for (i = 0; i < size - 1; i++) {
        ch = synchconsole->SynchGetChar();

        if (ch == EOF)
            break;

        ASSERT(machine->WriteMem(addr + i, 1, ch));

        if (ch == '\n' || ch == '\0') {
            i++;
            break;
        }
    }

    ASSERT(machine->WriteMem(addr + i, 1, '\0'));
    return true;
}

bool syscall_putint(void) {
    int val = PARAM(1);
    synchconsole->SynchPutInt(val);
    return true;
}

bool syscall_getint(void) {
    int val;
    int addr = PARAM(1);
    synchconsole->SynchGetInt(&val);
    machine->WriteMem(addr, sizeof(val), val);
    return true;
}

bool syscall_userthreadcreate(void) {
    int funcptr = PARAM(1);
    int argptr = PARAM(2);

    int ret = do_UserThreadCreate(funcptr, argptr);
    RETURN(ret);
    return true;
}

bool syscall_userthreadexit(void) {
    do_UserThreadExit();
    return true;
}

bool syscall_userthreadjoin(void) {
    int tid = PARAM(1);
    do_UserThreadJoin(tid);
    return true;
}

bool syscall_fork(void) {
    int pid;

    pid = processmanager->Fork();

    RETURN(pid);
    return true;
}

bool syscall_exec(void) {
    int sptr, ret;
    char filename[MAX_STRING_SIZE];

    sptr = PARAM(1);
    copyStringFromMachine(sptr, filename, MAX_STRING_SIZE);

    ret = processmanager->Exec(filename);

    RETURN(ret);
    return false;
}

bool syscall_forkexec(void) {
    int sptr, pid;
    char filename[MAX_STRING_SIZE];

    sptr = PARAM(1);
    copyStringFromMachine(sptr, filename, MAX_STRING_SIZE);

    pid = processmanager->ForkExec(filename);

    RETURN(pid);
    return (true);
}

bool syscall_waitpid(void) {
    int pid = PARAM(1);
    int ret = processmanager->WaitPid(pid);
    RETURN(ret);
    return true;
}

bool syscall_getpid(void) {
    RETURN(currentThread->pid);
    return true;
}

bool syscall_getppid(void) {
    int ppid = processmanager->getppid(currentThread->pid);

    /* The current thread is currently running
     * it MUST be in the process list of the process manager
     */
    ASSERT(ppid != -1);

    RETURN(ppid);
    return true;
}

bool syscall_sbrk() {
    int n = PARAM(1);
    int ret = currentThread->space->Sbrk(n);
    RETURN(ret);
    return true;
}

bool syscall_create(void) {
    int sptr;
    int size;
    char filename[MAX_STRING_SIZE];

    sptr = PARAM(1);
    copyStringFromMachine(sptr, filename, MAX_STRING_SIZE);

    size = PARAM(2);

    RETURN(fileSystem->Create(filename, size) != -1);
    return true;
}

bool syscall_open(void) {
    int sptr;
    int fd;
    char filename[MAX_STRING_SIZE];
    OpenFile *file;

    sptr = PARAM(1);
    copyStringFromMachine(sptr, filename, MAX_STRING_SIZE);

    fd = openFileTable->Reserve();
    if (fd == -1)
	    goto out;

    file = fileSystem->Open(filename);
    if (!file) {
	    fd = -1; goto out;
    }

    openFileTable->Set(fd, file);

out:
    RETURN(fd);
    return true;
}

bool syscall_close(void) {
    int fd;
    OpenFile *file;

    fd = PARAM(1);
    file = openFileTable->Get(fd);
    delete file;
    openFileTable->Release(fd);

    return true;
}

bool syscall_read(void) {
	int fd = -1, sptr, size;
	char *buf;
	OpenFile *file = NULL;

	sptr = PARAM(1);
	size = PARAM(2);
	fd = PARAM(3);

	if (size < 0)
		goto error;
	else if (size == 0) {
		RETURN(0);
		return true;
	}

	file = openFileTable->Get(fd);
	if (size == -1)
		goto error;

	buf = new char[size];
	if (!buf)
		goto error;
	RETURN(file->Read(buf, size));

	copyBufToMachine(buf, sptr, size);
	return true;
error:
	RETURN(-1);
	return true;
}

#endif /* SYSCALL_HANDLERS_H_ */
