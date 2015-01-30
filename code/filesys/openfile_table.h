#ifndef OPENFILE_TABLE_H
#define OPENFILE_TABLE_H

/*
 * Table of OpenFile pointers.
 *
 * Associate a integer (file descriptor) to a pointer to OpenFile.  This file
 * descriptor is used by userspace programs to refer to an OpenFile pointer.
 *
 * This table is also used to store those pointers and raise error when system
 * limit are reached.
 *
 * Multiple instance can be created as they don't share any data between instance.
 *
 * Invalid file descriptor is -1.
 */

#include "openfile.h"

class OpenFileTable {
public:
	OpenFileTable(unsigned size);
	~OpenFileTable(void);

	/*
	 * Return a valid file descriptor if possible.  Nothing is yet associated,
	 * a call to Set() must be performed later.
	 *
	 * On error, an invalid file descriptor is returned.
	 */
	int Reserve(void);

	/*
	 * Associate to the file descriptor fd the OpenFile pointer file.
	 *
	 * The file descriptor *must* be reserved somehow.
	 */
	void Set(int fd, OpenFile *file);

	/*
	 * Remove the file descriptor and his associated OpenFile pointer.
	 */
	void Release(int fd);

	/*
	 * Return the file descriptor associated to the OpenFile pointer.  The
	 * pointer must be in the table, otherwise, an invalid file descriptor
	 * is returned.
	 */
	int Get(OpenFile *file);

	/*
	 * Return the OpenFile pointer associated to the given file descriptor.
	 * If the file descriptor refer to nothing, NULL is returned.
	 */
	OpenFile *Get(int fd);

private:
	unsigned remain, size;
	OpenFile **table;
	bool IsEmpty(void);
	bool IsFull(void);
};

#endif // OPENFILE_TABLE_H
