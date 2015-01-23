#include "syscall.h"

int main() {
	if (!Create("test", 128)) {
		PutString("Error when creating file\n");
		return 1;
	}
	return 0;
}
