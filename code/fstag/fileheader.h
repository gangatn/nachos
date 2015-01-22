#ifndef FILEHEADER_H
#define FILEHEADER_H

class FileHeader {
	private:
		Inode inode;
		TagGroup taggroup;
public :
		FileHeader(TagGroup *tg);
		
};

#endif
