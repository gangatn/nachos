#include "openfile_table.h"

OpenFileTable::OpenFileTable(unsigned asize)
{
	unsigned i;

	this->remain = asize;
	this->size = asize;
	this->table = new OpenFile*[asize];

	for (i = 0; i < asize; i++)
		this->table[i] = NULL;
}

OpenFileTable::~OpenFileTable(void)
{
	delete [] table;
}


int OpenFileTable::Reserve(void)
{
	unsigned i = 0;

	if (IsFull())
		return -1;

	while (this->table[i] != NULL)
		i++;
	this->remain--;
	return i;
}


void OpenFileTable::Set(int fd, OpenFile *file)
{
	this->table[fd] = file;
}

void OpenFileTable::Release(int fd)
{
	if (fd < 0 || (unsigned)fd >= this->size)
		return;
	this->table[fd] = NULL;
	this->remain++;
}


int OpenFileTable::Get(OpenFile *file)
{
	unsigned i;

	if (IsEmpty())
		return -1;

	for (i = 0; i < this->size; i++)
		if (this->table[i] == file)
			break;
	if (i == this->size)
		return - 1;
	return i;
}


OpenFile *OpenFileTable::Get(int fd)
{
	if (fd < 0 || (unsigned)fd >= this->size)
		return NULL;
	return this->table[fd];
}


bool OpenFileTable::IsFull(void)
{
	return this->remain == 0;
}

bool OpenFileTable::IsEmpty(void)
{
	return this->remain == this->size;
}
