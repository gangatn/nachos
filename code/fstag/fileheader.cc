#include "fileheader.h"


FileHeader::FileHeader(TagGroup *tg)
{
	taggroup = tg;
	sector = SectorManager.get();
	inode = new Inode();
	
}
