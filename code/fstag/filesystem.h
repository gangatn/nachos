#ifndef FILESYS_H
#define FILESYS_H
#include "bitmap.h"
class Filesystem {
private;
	
public:
	int init();
	int create(char* name, char** tags);
	int remove(char* name, char** tags);
};

#endif // FILESYS_H
