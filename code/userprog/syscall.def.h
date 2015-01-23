/*
 * syscall.def.h:
 *
 * Features:
 * Easy definition of system calls.
 *
 * There is basically two macros to define a new system call:
 * SYSCALL_PROC and SYSCALL_FUNC which define respectively
 * a procedure system call (which returns void) and a function
 * macro which returns a value of a give type
 *
 * How to use theses macros:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * SYSCALL_PROC(name, handler, ...)
 * -> this will define a new procedure system call, named "name"
 * with the associated handler and ... represents system call parameters
 *
 * SYSCALL_FUNC(name, handler, type, ...)
 * -> this will define a new function system call, name "name"
 * with the associated handler and return a value of type "
 *
 * About handlers:
 * ~~~~~~~~~~~~~~~
 * Handler can be NULL, but this will generate an un handled system call
 * and behavior of user program may be wrong, or they are a pointer
 * to a handler function, usually defined in userprog/syscall_handlers.h
 *
 * Rationale:
 * ----------
 * The old system call system was not really convenient,
 * in order to add a new system call, we needed to:
 * - attribute an id and define with correct name
 * - add reference in start.S to avoid linking issues
 * - add a new case in exception.cc to handle the system call
 *
 * This was tedious and error prone.
 *
 * Note:
 * This make a massive use of an old programming technique called
 * X-Macros to automatie code generation.
 */

/* Include guard to ensure macro definition */
#ifndef SYSCALL_PROC
#error "syscalls: SYSCALL_PROC must be defined"
#endif

#ifndef SYSCALL_FUNC
#error "syscalls: SYSCALL_FUNC must be defined"
#endif

/*
 * TODO: Handle __attribute__((noreturn));???
 */
SYSCALL_PROC(Halt, syscall_halt)
SYSCALL_PROC(Exit, syscall_exit, int errcode)

SYSCALL_FUNC(Create, syscall_create, int, char *name, int size)
SYSCALL_FUNC(Open, NULL, int, char *name)
SYSCALL_PROC(Write, NULL, char *buffer, int size, int file_id)
SYSCALL_FUNC(Read, NULL, int, char *buffer, int size, int file_id)
SYSCALL_PROC(Close, NULL, int file_id)

SYSCALL_PROC(PutChar, syscall_putchar, char c)
SYSCALL_PROC(PutString, syscall_putstring, char *str)
SYSCALL_FUNC(GetChar, syscall_getchar, int)
SYSCALL_PROC(GetString, syscall_getstring, char *s, int n)
SYSCALL_PROC(PutInt, syscall_putint, int n)
SYSCALL_PROC(GetInt, syscall_getint, int *n)

SYSCALL_FUNC(UserThreadCreate, syscall_userthreadcreate, int, void (*f)(void*), void* arg)
SYSCALL_PROC(UserThreadExit, syscall_userthreadexit)
SYSCALL_PROC(UserThreadJoin, syscall_userthreadjoin, int tid)

SYSCALL_FUNC(Fork, syscall_fork, int)
SYSCALL_FUNC(Exec, syscall_exec, int, char *s)
SYSCALL_FUNC(ForkExec, syscall_forkexec, int, char *filename)
SYSCALL_FUNC(WaitPid, syscall_waitpid, int, int pid)
SYSCALL_FUNC(getpid, syscall_getpid, int)
SYSCALL_FUNC(getppid, syscall_getppid, int)

SYSCALL_PROC(Yield, NULL)
SYSCALL_PROC(Join, NULL)

SYSCALL_FUNC(Sbrk, syscall_sbrk, void *, int n)
