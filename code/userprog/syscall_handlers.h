#ifndef SYSCALL_HANDLERS_H_
#define SYSCALL_HANDLERS_H_


bool syscall_halt(void);
bool do_syscall_exit(int s);
bool syscall_exit(void);
bool syscall_putchar(void);
bool syscall_putstring(void);
bool syscall_getchar(void);
bool syscall_getstring(void);
bool syscall_putint(void);
bool syscall_getint(void);
bool syscall_userthreadcreate(void);
bool syscall_userthreadexit(void);
bool syscall_userthreadjoin(void);
bool syscall_fork(void);
bool syscall_exec(void);
bool syscall_forkexec(void);
bool syscall_waitpid(void);
bool syscall_getpid(void);
bool syscall_getppid(void);
bool syscall_create(void);
bool syscall_open(void);
bool syscall_close(void);

bool syscall_sbrk(void);
#endif /* SYSCALL_HANDLERS_H_ */
