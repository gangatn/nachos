#include "syscall.h"

/*
 * we use some tricks here that needs to be cleared:
 *
 * First of all, we keep arguments, so the mips c compiler
 * will automatically put arguments into register, and the handler
 * can retrieve them later
 *
 * We must use the "volatile" keyword, indeed, after executing
 * "syscall", the corresponding handler will be triggered by the kernel
 * but the c compiler does not know that, if he optimize agressively the code
 * the inlined asm might be removed since the compiler does not find any
 * results of the code.
 *
 * The third trick we use is the asm input operands to get the system call id
 */

#define SYSCALL_PROC(name, handler, ...)  \
	void name ( __VA_ARGS__ ) \
	{ \
	int syscall = SC_##name; \
	asm volatile ( "addu $2, $0, %0" : : "r"(syscall)); \
	asm volatile ( "syscall" ); \
	}

#define SYSCALL_FUNC(name, handler, type, ...) \
	type name ( __VA_ARGS__ ) \
	{ \
	int syscall = SC_##name; \
	register type cache_value __asm__("$2"); \
	asm volatile ( "addu $2, $0, %0" : : "r"(syscall)); \
	asm volatile ( "syscall" ); \
	return cache_value; \
	}

#include "syscall.def.h"

#undef SYSCALL_PROC
#undef SYSCALL_FUNC
