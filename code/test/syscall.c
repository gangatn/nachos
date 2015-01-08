#include "syscall.h"


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
