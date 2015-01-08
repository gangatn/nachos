/*
 *
 */


/*
 * TODO: Handle __attribute__((noreturn));???
 */
SYSCALL_PROC(Halt, syscall_halt)
SYSCALL_PROC(Exit, syscall_exit, int errcode)
SYSCALL_FUNC(Exec, NULL, int, char *name)
SYSCALL_FUNC(Join, NULL, int, int id)
SYSCALL_PROC(Create, NULL, char *name)
SYSCALL_FUNC(Open, NULL, int, char *name)
SYSCALL_PROC(Write, NULL, char *buffer, int size, int file_id)
SYSCALL_FUNC(Read, NULL, int, char *buffer, int size, int file_id)
SYSCALL_PROC(Close, NULL, int file_id)
SYSCALL_PROC(Fork, NULL, void (*func)(void))
SYSCALL_PROC(Yield, NULL)

SYSCALL_PROC(PutChar, syscall_putchar, char c)
SYSCALL_PROC(PutString, syscall_putstring, char *str)
SYSCALL_FUNC(GetChar, syscall_getchar, int)
SYSCALL_PROC(GetString, syscall_getstring, char *s, int n)
SYSCALL_PROC(PutInt, syscall_putint, int n)
SYSCALL_PROC(GetInt, syscall_getint, int *n)
