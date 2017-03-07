/**
 * Name: eAddresses.c
 * Desc: Shows how to use the LJM_eAddresses function
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

	// Write 2.5V to DAC0,
	// write 12345 to TEST_UINT16,
	// read TEST_UINT16,
	// read serial number,
	// read product ID,
	// and read firmware version.

	#define NUM_FRAMES 6

	int ADDRESSES[NUM_FRAMES] = {1000, 55110, 55110, 60028, 60000, 60004};
	int TYPES[NUM_FRAMES] = {LJM_FLOAT32, LJM_UINT16, LJM_UINT16, LJM_UINT32,
	                         LJM_FLOAT32, LJM_FLOAT32};
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
			printf(" address %d (type: %d)\n", ADDRESSES[frameI], TYPES[frameI]);
		}
		else {
			valueI += NUM_VALUES[frameI];
		}
	}

	err = LJM_eAddresses(handle, NUM_FRAMES, ADDRESSES, TYPES, WRITE_READS, NUM_VALUES,
		values, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eAddresses");

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
			printf(" from address %d (type: %d)\n", ADDRESSES[frameI], TYPES[frameI]);
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
