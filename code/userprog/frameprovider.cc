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
	return bitmap.Find();
}


void FrameProvider::ReleaseFrame(int frame)
{
	bitmap.Clear(frame);
}


int FrameProvider::NumAvailFrame()
{
	return bitmap.NumClear();
}
