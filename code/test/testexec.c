#include "syscall.h"

int main()
{
	PutString("Let's exec\n");
	Exec("putstring");
	PutString("NEVER REACHED\n");

	return 0;
}
