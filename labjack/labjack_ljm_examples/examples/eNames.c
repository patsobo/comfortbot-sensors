/**
 * Name: eNames.c
 * Desc: Shows how to use the LJM_eNames function
**/

// For printf
#include <stdio.h>

// For the LabJackM library
#include "LabJackM.h"

// For LabJackM helper functions
#include "LJM_Utilities.h"

int main()
{
	int err, frameI, arrayI, valueI, handle;
	int errorAddress = INITIAL_ERR_ADDRESS;

	#define NUM_FRAMES 6

	const char * NAMES[NUM_FRAMES] = {"DAC0", "TEST_UINT16", "TEST_UINT16", "SERIAL_NUMBER",
	                                  "PRODUCT_ID", "FIRMWARE_VERSION"};
	int WRITE_READS[NUM_FRAMES] = {LJM_WRITE, LJM_WRITE, LJM_READ, LJM_READ,
	                         LJM_READ, LJM_READ};
	int NUM_VALUES[NUM_FRAMES] = {1, 1, 1, 1, 1, 1};
	double values[NUM_FRAMES] = {2.5, 12345, 0.0, 0.0, 0.0, 0.0};

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Print what we're writing
	valueI = 0;
	printf("Writing:\n");
	for (frameI=0; frameI<NUM_FRAMES; frameI++) {
		if (WRITE_READS[frameI] == LJM_WRITE) {
			printf("\t");
			for (arrayI=0; arrayI<NUM_VALUES[frameI]; arrayI++) {
				printf("%f ", values[valueI++]);
			}
			if (NUM_VALUES[frameI] > 1) {
				printf(" as array starting at");
			}
			else {
				printf(" to");
			}
			printf(" %s\n", NAMES[frameI]);
		}
		else {
			valueI += NUM_VALUES[frameI];
		}
	}

	err = LJM_eNames(handle, NUM_FRAMES, NAMES, WRITE_READS, NUM_VALUES,
		values, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames");

	// Print what we've read
	valueI = 0;
	printf("Read:\n");
	for (frameI=0; frameI<NUM_FRAMES; frameI++) {
		if (WRITE_READS[frameI] == LJM_READ) {
			printf("\t");
			for (arrayI=0; arrayI<NUM_VALUES[frameI]; arrayI++) {
				printf("%f ", values[valueI++]);
			}
			if (NUM_VALUES[frameI] > 1) {
				printf(" as array starting");
			}
			printf(" from %s\n", NAMES[frameI]);
		}
		else {
			valueI += NUM_VALUES[frameI];
		}
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
