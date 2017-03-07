/**
 * Name: eWriteAddresses.c
 * Desc: Shows how to use the LJM_eWriteAddresses function
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

	// 2.5V to DAC0 and write 12345 to TEST_UINT16 (55110)

	#define NUM_FRAMES 2

	int ADDRESSES[NUM_FRAMES] = {1000, 55110};
	int TYPES[NUM_FRAMES] = {LJM_FLOAT32, LJM_UINT16};
	double VALUES[NUM_FRAMES] = {2.5, 12345};

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Print what we're doing here
	printf("Writing:\n");
	for (i=0; i<NUM_FRAMES; i++) {
		printf("\t%f to address %d (type: %d)\n", VALUES[i], ADDRESSES[i], TYPES[i]);
	}

	err = LJM_eWriteAddresses(handle, NUM_FRAMES, ADDRESSES, TYPES, VALUES, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eWriteAddresses");

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
