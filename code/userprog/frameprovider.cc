#include "frameprovider.h"
#include "system.h"
#include <strings.h>


FrameProvider::FrameProvider()
{
	frame_count = NumPhysPages;
	frame_ref = new short[frame_count];
	memset(frame_ref, 0, sizeof(*frame_ref) * frame_count);
}

FrameProvider::~FrameProvider()
{
	delete [] frame_ref;
}


int FrameProvider::findFreeFrame()
{
	int i;
	int found = -1;

	for(i = 0; i < frame_count; i++)
	{
		if(frame_ref[i] == 0)
		{
			found = i;
			break;
		}
	}

	return found;
}

void FrameProvider::UseFrame(int frame)
{
	// We use an assert, if an invalid frame is provided
	// this is most likely a programming error
	// (but not always, memory corruption can eventually
	//  trigger this kind of issue)
	ASSERT(frame >= 0 && frame < frame_count);
	frame_ref[frame]++;
}

int FrameProvider::GetEmptyFrame()
{
	int frame = findFreeFrame();

	if (frame >= 0)
	{
		UseFrame(frame);
		/* We use memset, bzero is deprecated */
		memset(&machine->mainMemory[frame*PageSize], 0, PageSize);
	}
	return frame;
}

int FrameProvider::GetCopiedFrame(int copied_frame)
{
	int frame = findFreeFrame();

	if (frame >= 0)
	{
		UseFrame(frame);
		memcpy(&machine->mainMemory[frame*PageSize], &machine->mainMemory[copied_frame*PageSize], PageSize);
	}
	return frame;
}


void FrameProvider::ReleaseFrame(int frame)
{
	ASSERT(frame >= 0 && frame < frame_count);
	ASSERT(frame_ref[frame] > 0);

	frame_ref[frame]--;
}


int FrameProvider::NumAvailFrame()
{
	int i;
	int avail = 0;

	// TODO: This implementation is currently
	// inefficient, this can be improved
	// using size counters
	for (i = 0; i < frame_count; i++)
	{
		if(frame_ref[i ] == 0) avail++;
	}

	return avail;
}
