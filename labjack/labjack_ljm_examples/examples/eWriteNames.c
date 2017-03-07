/**
 * Name: eWriteNames.c
 * Desc: Shows how to use the LJM_eWriteNames function
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include "LabJackM.h"

// For LabJackM helper functions
#include "LJM_Utilities.h"

int main()
{
	int err, i, handle;
	int errorAddress = INITIAL_ERR_ADDRESS;

	#define NUM_FRAMES 2

	const char * NAMES[NUM_FRAMES] = {"DAC0", "TEST_UINT16"};
	double VALUES[NUM_FRAMES] = {2.5, 12345};

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Print what we're doing here
	printf("Writing:\n");
	for (i=0; i<NUM_FRAMES; i++) {
		printf("\t%f to %s\n", VALUES[i], NAMES[i]);
	}

	err = LJM_eWriteNames(handle, NUM_FRAMES, NAMES, VALUES, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eWriteNames");

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
