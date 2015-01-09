#ifndef SYSCALL_HANDLERS_H_
#define SYSCALL_HANDLERS_H_


void syscall_halt(void);
void syscall_exit(void);
void syscall_putchar(void);
void syscall_putstring(void);
void syscall_getchar(void);
void syscall_getstring(void);
void syscall_putint(void);
void syscall_getint(void);
void syscall_userthreadcreate(void);
void syscall_userthreadexit();


#endif /* SYSCALL_HANDLERS_H_ */
