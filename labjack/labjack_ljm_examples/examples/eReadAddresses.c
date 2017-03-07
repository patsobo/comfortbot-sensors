/**
 * Name: eReadAddresses.c
 * Desc: Shows how to use the LJM_eReadAddresses function
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

	// Read serial number, product ID, and then firmware version.

	#define NUM_FRAMES 3

	int ADDRESSES[NUM_FRAMES] = {60028, 60000, 60004};
	int TYPES[NUM_FRAMES] = {LJM_UINT32, LJM_FLOAT32, LJM_FLOAT32};
	double values[NUM_FRAMES] = {0.0, 0.0, 0.0};

	int errorAddress = INITIAL_ERR_ADDRESS;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	err = LJM_eReadAddresses(handle, NUM_FRAMES, ADDRESSES, TYPES, values, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadAddresses");

	// Print results
	for (i = 0; i<NUM_FRAMES; i++) {
		printf("Address - %d, type - %d: %f\n", ADDRESSES[i], TYPES[i], values[i]);
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
