// frameprovider.h
//
// Physical frame provider, used by virtual memory mechanism
// i.e. paging to associate physical memory pages to
// a virtual page
//
// Our implementation uses a reference counter instead of a bitmap
// this permit us to allow multiple user for one given frame
//
#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H



class FrameProvider {
public:
    FrameProvider();
    ~FrameProvider();

    // GetEmptyFrame:
    //
    // Return a valid physical empty frame
    // this frame shall not be returned
    // again by multiple calls until released
    //
    // All bytes of the frame equals 0
    //
    // This also mark the returned frame as used
    //
    // Return -1 if no frame is available
    int GetEmptyFrame();

    // GetCopiedFrame:
    //
    // Return a valid physical frame that is
    // a copy of the given frame
    //
    // This also mark the returned frame as used
    //
    // Return -1 if no frame is available
    int GetCopiedFrame(int frame);

    // UseFrame:
    //
    // Indicated the frame provider that there
    // is a new user to the given thread
    // This is *OBLIGATORY* to call this function
    // if you use a frame that you did not get using
    // the GetEmptyFrame or the GetCopiedFrame functions
    void UseFrame(int frame);

    // Release the given frame
    // This frame shall not be used again
    // by the called
    void ReleaseFrame(int frame);

    // Returns the number of available frames
    int NumAvailFrame();
private:
    short *frame_ref;
    int frame_count;

    int findFreeFrame();
};


#endif /* FRAMEPROVIDER_H */
