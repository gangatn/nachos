#include <stdlib.h>
#include <assert.h>
#include "syscall.h"

void __assert_fail(const char *assertion, const char *file,
				   unsigned line)
{
	PutString((char*)file);
	PutChar(':');
	PutInt(line);
	PutString(": ");
	PutString("Assertion `");
	PutString((char*)assertion);
	PutString("' failed.\n");
	abort();
}
