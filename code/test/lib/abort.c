#include <stdlib.h>
#include <syscall.h>

/*
 * In posix, we use the the raise function
 * (which is a call to the kill system call)
 *
 * Since we don't have signals yet, we will use
 * Exit() system call
 * */

void abort(void)
{
	PutString("Aborted.\n");
	Exit(1);
}
