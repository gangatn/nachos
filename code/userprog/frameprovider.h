// frameprovider.h
//
// Physical frame provider, used by virtual memory mechanism
// i.e. paging to associate physical memory pages to
// a virtual page
#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"

class FrameProvider
{
public:
	FrameProvider();
	~FrameProvider();

	// Return a valid physical empty frame
	// this frame shall not be returned
	// again by multiple calls until released
	//
	// All bytes of the frame equals 0
	int GetEmptyFrame();

	// Return a valid physical frame that is
	// a copy of the given frame
	int GetCopiedFrame(int frame);

	// Release the given frame
	// This frame shall not be used again
	// by the called
	void ReleaseFrame(int frame);

	// Returns the number of available frames
	int NumAvailFrame();
private:
	BitMap bitmap;
};


#endif /* FRAMEPROVIDER_H */
