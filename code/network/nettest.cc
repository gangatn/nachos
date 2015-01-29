// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    const char *data = "Hello there!";
    const char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    postOffice->Send(outPktHdr, outMailHdr, data); 

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    postOffice->Send(outPktHdr, outMailHdr, ack); 

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}


// data are send on box 0 and ack on box 1
void MailTestRing(NetworkAddress idMachine, int nbMachine) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	const char *data = "\"Hello there!\"";
	const char *ack = "\"Got it!\"";
	char buffer[MaxMailSize];
	
	if(idMachine < 0 || idMachine >= nbMachine) {
		printf("The machine id is not valid!\n");
	}
	

	ASSERT(nbMachine > 1);

	if(idMachine == 0) {

		// Send the first message to the next machine on boxe 0
		outPktHdr.to = 1;
		outMailHdr.to = 0;
		outMailHdr.from = 0;
		outMailHdr.length = strlen(data) + 1;
	
		postOffice->Send(outPktHdr,outMailHdr, data);
		printf("Machine %d has sent data %s to machine %d (box %d -> %d) \n",idMachine, data, outPktHdr.to, outMailHdr.from, outMailHdr.to);
		
		// Wait for acknowledgement on boxe 1
		postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
		printf("Machine %d has got ack %s from machine %d (box %d <- %d) \n",idMachine, buffer, inPktHdr.from, inMailHdr.to, inMailHdr.from);

		// wait message from last machine
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		printf("Machine %d has got data %s from machine %d (box %d <- %d) \n",idMachine, buffer, inPktHdr.from, inMailHdr.to, inMailHdr.from);
		// Send the first message to the next machine on boxe 0
		outPktHdr.to = nbMachine -1;
		outMailHdr.to = 1;
		outMailHdr.from = 1;
		outMailHdr.length = strlen(ack) + 1;
		
		// Send the ack to next machine on boxe 1
		postOffice->Send(outPktHdr,outMailHdr, ack);
		printf("Machine %d has sent ack %s to machine %d (box %d -> %d) \n", idMachine, ack, outPktHdr.to, outMailHdr.from, outMailHdr.to);

	} else {

		// Get Data from previous machine
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		printf("Machine %d has got data %s from machine %d (box %d <- %d) \n",idMachine, buffer, inPktHdr.from, inMailHdr.to, inMailHdr.from);
		fflush(stdout);
		
		// Prebuild ack
		outPktHdr.to = (idMachine - 1) % nbMachine;
		outMailHdr.to = 1;
		outMailHdr.from = 1;
		outMailHdr.length = strlen(ack) + 1;
		
		// Send the ack to pevious machine on boxe 1
		postOffice->Send(outPktHdr,outMailHdr, ack);
		printf("Machine %d has sent ack %s to machine %d (box %d -> %d) \n", idMachine, ack, outPktHdr.to, outMailHdr.from, outMailHdr.to);
		fflush(stdout);
		
		// send data to the next machine
		outPktHdr.to = (idMachine + 1) % nbMachine;
		outMailHdr.to = 0;
		outMailHdr.from = 0;
		outMailHdr.length = strlen(data) + 1;

		postOffice->Send(outPktHdr,outMailHdr, data);
		printf("Machine %d has sent data %s to machine %d (box %d -> %d) \n", idMachine, data, outPktHdr.to, outMailHdr.from, outMailHdr.to);
		fflush(stdout);
	
		// Wait for acknowledgement on boxe 1
		postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
		printf("Machine %d has got ack %s from machine %d (box %d <- %d) \n",idMachine, buffer, inPktHdr.from, inMailHdr.to, inMailHdr.from);
		fflush(stdout);
	}
	
	// Then we're done!
	interrupt->Halt();
	
}
