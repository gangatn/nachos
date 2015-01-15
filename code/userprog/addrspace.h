// addrspace.h
//      Data structures to keep track of executing user programs
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		262144	// increase this as necessary!

class AddrSpace
{
public:
    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
#ifdef CHANGED
    // Create an address space from an existing one (given as space
    // parameter), if copy_on_write equals true, then
    // both all pages of both address space are switch in copy on write mode
    // This means that if a page is written, this will generate a pagefault
    // and the frame will be copied for write use
	AddrSpace (AddrSpace *space, bool copy_on_write);
#endif

    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

#ifdef CHANGED
	// HandleReadOnly:
	//
	// This function should be called when we get a ReadOnlyException
	// This function will determines wheter the fault occur in a copy
	// on write virtual page, if so, this will get a new physical frame
	// copied from the read only frame and return true
	//
	// If the ReadOnlyException occur in a location where the program
	// isn't supposed to write, nothing is done and the function return
	// false
	bool HandleReadOnly(int virt_addr);
#endif // CHANGED

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch

private:
	TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!
    unsigned int numPages;	// Number of pages in the virtual
    // address space
};

#endif // ADDRSPACE_H
