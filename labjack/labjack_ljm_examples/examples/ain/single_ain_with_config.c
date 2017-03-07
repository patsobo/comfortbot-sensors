/**
 * Name: single_ain_with_config.c
 * Desc: Demonstrates configuring and reading a single analog input (AIN) with a LabJack.
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include "LabJackM.h"

// For LabJackM helper functions
#include "../LJM_Utilities.h"

int main()
{
	int err;
	int handle;
	int i;

	// Set up for configuring the AIN
	enum { NUM_FRAMES_CONFIG = 4 };
	const char * aNamesConfig[NUM_FRAMES_CONFIG] = \
		{"AIN0_NEGATIVE_CH", "AIN0_RANGE", "AIN0_RESOLUTION_INDEX", "AIN0_SETTLING_US"};
	const double aValuesConfig[NUM_FRAMES_CONFIG] = {199, 10, 0, 0};
	int errorAddress = INITIAL_ERR_ADDRESS;

	// Set up for reading AIN value
	double Value = 0;
	const char * Name = "AIN0";

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Setup and call eWriteNames to configure AINs on the LabJack.
	err = LJM_eWriteNames(handle, NUM_FRAMES_CONFIG, aNamesConfig, aValuesConfig,
		&errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eWriteNames");

	printf("\nSet configuration:\n");
	for (i=0; i<NUM_FRAMES_CONFIG; i++) {
		printf("    %s : %f\n", aNamesConfig[i], aValuesConfig[i]);
	}

	// Read AIN from the LabJack
	err = LJM_eReadName(handle, Name, &Value);
	ErrorCheck(err, "LJM_eReadName");

	// Print results
	printf("%s : %f V\n", Name, Value);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
