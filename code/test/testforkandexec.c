#include "syscall.h"

int main(void)
{
	int pid = Fork();

	if (pid == 0)
		Exec("putstring");
	else {
		// PLZ WAIT FOR pid
	}

	return 0;
}
