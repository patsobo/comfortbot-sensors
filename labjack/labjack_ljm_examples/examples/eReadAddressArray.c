/**
 * Name: eReadAddressArray.c
 * Desc: Shows how to use the LJM_eReadAddressArray function
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

	#define NUM_VALUES 3

	// AIN0 is address 0
	const int ADDRESS_ARRAY_START = 0;
	const int ADDRESS_TYPE = LJM_FLOAT32;
	const int ADDRESSES_PER_FLOAT32 = 2;
	double values[NUM_VALUES];

	int errorAddress = INITIAL_ERR_ADDRESS;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);
	printf("\nLJM_eReadAddressArray(Handle=%d, Address=%d, Type=%d, NumValues=%d, ...):\n", handle,
		ADDRESS_ARRAY_START, ADDRESS_TYPE, NUM_VALUES);

	err = LJM_eReadAddressArray(handle, ADDRESS_ARRAY_START, ADDRESS_TYPE, NUM_VALUES, values,
		&errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadAddressArray");

	// Print results
	for (i = 0; i < NUM_VALUES; i++) {
		printf("Address: %d, Type: %d, value: %f\n",
			ADDRESS_ARRAY_START + (i * ADDRESSES_PER_FLOAT32), ADDRESS_TYPE, values[i]);
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
