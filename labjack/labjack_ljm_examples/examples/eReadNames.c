/**
 * Name: eReadNames.c
 * Desc: Shows how to use the LJM_eReadNames function
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

	#define NUM_FRAMES 3

	const char * NAMES[NUM_FRAMES] = {"SERIAL_NUMBER", "PRODUCT_ID", "FIRMWARE_VERSION"};
	double values[NUM_FRAMES] = {0.0, 0.0, 0.0};

	int errorAddress = INITIAL_ERR_ADDRESS;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	err = LJM_eReadNames(handle, NUM_FRAMES, NAMES, values, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadNames");

	// Print results
	for (i = 0; i<NUM_FRAMES; i++) {
		printf("%s: %f\n", NAMES[i], values[i]);
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
