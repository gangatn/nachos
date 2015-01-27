#include "syscall.h"
#define BUFFER_SIZE 80

int main () {
	char buffer[BUFFER_SIZE];
	char prompt[3];
	int i;
	int pid;

	// Prompt theme //
	prompt[0] = '-';
	prompt[1] = '>';
	prompt[2] = 0;

	buffer[0] = 0;

	PutString("Hi Dude !\n");
	while(1) {
		PutString(prompt);

		i = 0;
		buffer[i] = GetChar(); // get first character
		
		if(buffer[i] == '\n') { // Go to new line
			continue;
		}
		
		if(buffer[i] == -1) { // Exit
			PutChar('\n');
			break;
		}
		do { // Get command line
			i++;
		} while((buffer[i % BUFFER_SIZE] = GetChar()) != '\n');
		
		if(i >= BUFFER_SIZE) {
			PutString("Command too long!\n");
			continue;
		}
		
		buffer[i] = 0; // remove the '\n'
		

		pid = ForkExec(buffer);
		if(pid == -1)
		{
			PutString("Command not found!\n");
		}
		else
		{
			WaitPid(pid);
		}
	}
	PutString("See U BRO !\n");
	return 0;
}
