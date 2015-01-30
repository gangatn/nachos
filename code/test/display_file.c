#include "syscall.h"

#define FILENAME "small"

int main() {
	int fd, ret;
	char buf[20];

	fd = Open(FILENAME);
	if (fd == -1) {
		PutString("Error when opening file \"" FILENAME "\"\n");
		return 1;
	}

	ret = Read(buf, 20, fd);
	if (ret != 20) {
		PutString("Error when opening file \"" FILENAME "\"\n");
		return 1;
	}

	buf[19] = '\0';
	PutString(buf);

	Close(fd);
	return 0;
}
