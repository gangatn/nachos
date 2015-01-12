#include "frameprovider.h"
#include "system.h"
#include <strings.h>


FrameProvider::FrameProvider()
	: bitmap(NumPhysPages)
{

}

FrameProvider::~FrameProvider()
{

}


int FrameProvider::GetEmptyFrame()
{
	int frame = bitmap.Find();
	bzero(&machine->mainMemory[frame*PageSize], PageSize);
	return frame;
}

int FrameProvider::GetCopiedFrame(int copied_frame)
{
	int frame = bitmap.Find();
	memcpy(&machine->mainMemory[frame], &machine->mainMemory[copied_frame], PageSize);
	return frame;
}


void FrameProvider::ReleaseFrame(int frame)
{
	bitmap.Clear(frame);
}


int FrameProvider::NumAvailFrame()
{
	return bitmap.NumClear();
}