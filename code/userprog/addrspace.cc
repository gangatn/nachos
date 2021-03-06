// addrspace.cc
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader * noffH) {
    noffH->noffMagic = WordToHost (noffH->noffMagic);
    noffH->code.size = WordToHost (noffH->code.size);
    noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
    noffH->initData.size = WordToHost (noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost (noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost (noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}

#ifdef CHANGED

static void ReadAtVirtual(OpenFile *executable, int virtualaddr,
                          int numBytes, int position,
                          TranslationEntry *pageTable, unsigned numPages) {
    TranslationEntry *saved_page_table = machine->pageTable;
    int saved_page_table_size = machine->pageTableSize;
    int i;
    char *mem_buffer;
    int written_data;


    // We need to *temporarly* change the machine pageTable and pageTableSize
    // because when we call WriteMem, this will use the current pageTable
    // and pageTableSize
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

    // If no memory is available, C++ will raise an exception
    // We don't deal with this kind of issue here
    mem_buffer = new char[numBytes];
    executable->ReadAt(mem_buffer, numBytes, position);


    // For obvious efficiency reason we write the buffer
    // word by word, if the data size is not a multiple
    // of 4 bytes, we write the remaining data
    for (i = 0; i < numBytes; i += 4) {
        machine->WriteMem(virtualaddr + i, 4, *((int*)&mem_buffer[i]));
    }

    written_data = (numBytes / 4) * 4;

    for (i = 0; i < numBytes % 4; i++) {
        machine->WriteMem(virtualaddr + written_data + i, 1,
                          mem_buffer[written_data + i]);
    }

    // We recover old machine state
    machine->pageTable = saved_page_table;
    machine->pageTableSize = saved_page_table_size;

    delete [] mem_buffer;
}

#endif /* CHANGED */

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable) {
    NoffHeader noffH;
    unsigned int i, size;
#ifdef CHANGED
    unsigned int endBss, stackLimit;

#endif // CHANGED
    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost (noffH.noffMagic) == NOFFMAGIC))
        SwapHeader (&noffH);
    ASSERT (noffH.noffMagic == NOFFMAGIC);
#ifdef CHANGED
    endBss = noffH.initData.virtualAddr + noffH.initData.size + noffH.uninitData.size;
    stackLimit =  endBss + UserHeapSize;
    brk =  endBss;
    DEBUG('a', "brk init to %d\n", brk);
#endif // CHANGED

    // how big is address space?
#ifndef CHANGED
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize ;
#else
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserHeapSize + UserStackSize ;
#endif
    // we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp (size, PageSize);
    size = numPages * PageSize;

    ASSERT (numPages <= NumPhysPages);  // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG ('a', "Initializing address space, num pages %d, size %d\n",
           numPages, size);
    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i; // for now, virtual page # = phys page #
#ifndef CHANGED
        pageTable[i].physicalPage = i;
        pageTable[i].valid = TRUE;
#else // CHANGED
        // If the whole page is in the heap, we invalidate the page
        if ((i * PageSize >= endBss && (i + 1) * PageSize < stackLimit)) {
            pageTable[i].valid = false;
        } else {
            pageTable[i].valid = true;
            pageTable[i].physicalPage = frameprovider->GetEmptyFrame();
        }
#endif
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;    // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
    }

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
               noffH.code.virtualAddr, noffH.code.size);
#ifndef CHANGED
        executable->ReadAt (&(machine->mainMemory[noffH.code.virtualAddr]),
                            noffH.code.size, noffH.code.inFileAddr);
#else
        ReadAtVirtual(executable, noffH.code.virtualAddr,
                      noffH.code.size, noffH.code.inFileAddr,
                      pageTable, numPages);
#endif
    }
    if (noffH.initData.size > 0) {
        DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
               noffH.initData.virtualAddr, noffH.initData.size);

#ifndef CHANGED
        executable->ReadAt (&
                            (machine->mainMemory
                             [noffH.initData.virtualAddr]),
                            noffH.initData.size, noffH.initData.inFileAddr);
#else
        ReadAtVirtual(executable, noffH.initData.virtualAddr,
                      noffH.initData.size, noffH.initData.inFileAddr,
                      pageTable, numPages);
#endif
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace () {
#ifdef CHANGED
    unsigned int i;
    // We don't forget to release the frames
    for (i = 0; i < numPages; i++) {
        if (pageTable[i].valid) {
            frameprovider->ReleaseFrame(pageTable[i].physicalPage);
        }
    }
#endif // CHANGED
    // LB: Missing [] for delete
    // delete pageTable;
    delete [] pageTable;
    // End of modification
}


#ifdef CHANGED

AddrSpace::AddrSpace (AddrSpace *space, bool copy_on_write) {
    unsigned int i;

    numPages = space->numPages;
    pageTable = new TranslationEntry[numPages];

    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;

        pageTable[i].valid = space->pageTable[i].valid;
        pageTable[i].use = space->pageTable[i].use;
        pageTable[i].dirty = space->pageTable[i].dirty;

        if (pageTable[i].valid) {
            if (copy_on_write) {
                pageTable[i].physicalPage = space->pageTable[i].physicalPage;
                frameprovider->UseFrame(pageTable[i].physicalPage);
            } else {
                pageTable[i].physicalPage =
                    frameprovider->GetCopiedFrame(
                        space->pageTable[i].physicalPage);
            }
        }

        if (copy_on_write) {
            pageTable[i].readOnly = true;
            space->pageTable[i].readOnly = true;
        } else {
            pageTable[i].readOnly = space->pageTable[i].readOnly;
        }
    }
}

bool AddrSpace::HandleReadOnly(int vaddr) {
    unsigned vp = vaddr / PageSize;

    if (pageTable[vp].valid && vp >= 0 && vp < numPages) {
        int new_frame = frameprovider->GetCopiedFrame(
                            pageTable[vp].physicalPage);

        // JD: Maybe we should handle a special case when we don't have memory
        // anymore? maybe use swap but this should IMHO be integrated in the frameprovider
        //
        // For now we just return false but this kind of error should not be ignored
        //
        // Since we don't have an heap yet, and code is not read only
        // We assume that we are in a CopyOnWrite case

        if (new_frame >= 0) {
            // We tell the frame provider that we no longer use this frame
            // and we set the freshly copied frame
            // We do not forget to permit write
            frameprovider->ReleaseFrame(pageTable[vp].physicalPage);
            pageTable[vp].physicalPage = new_frame;
            pageTable[vp].readOnly = false;
            return true;
        }
    }
    return false;
}

int AddrSpace::Sbrk(unsigned int n) {
    unsigned int i, oldBrk, newBrk;
    unsigned int startPage;
    unsigned int endPage;

    oldBrk = brk; // save the old brk to return
    newBrk = brk + n;
    // if there is not enough space available in the heap -> error
    if (brk + n >= numPages * PageSize -  UserStackSize)
        return -1;

    startPage = brk / PageSize;
    endPage = newBrk / PageSize;

    for (i = startPage; i <= endPage; ++i) {
        if (!pageTable[i].valid) {
            int frame = frameprovider->GetEmptyFrame();
            if (frame == -1)
                return -1;
            pageTable[i].valid = true;
            pageTable[i].physicalPage = frame;
        }
    }

    brk = newBrk;
    return oldBrk;
}
#endif

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters () {
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister (i, 0);

    // Initial program counter -- must be location of "Start"
    // TO BE SURE
    machine->WriteRegister (PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister (NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister (StackReg, numPages * PageSize - 16);
    DEBUG ('a', "Initializing stack register to %d\n",
           numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void
AddrSpace::SaveState () {
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState () {
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
