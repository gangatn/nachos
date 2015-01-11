#include "frameprovider.h"


FrameProvider::FrameProvider(int frame_count)
	: bitmap(frame_count)
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
