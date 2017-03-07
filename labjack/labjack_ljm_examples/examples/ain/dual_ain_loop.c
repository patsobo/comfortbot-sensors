/**
 * Name: dual_ain_loop.c
 * Desc: Demonstrates reading 2 analog inputs (AINs) in a loop from a LabJack.
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
	int errorAddress = INITIAL_ERR_ADDRESS;

	// Set up for configuring the AINs
	enum { NUM_FRAMES_CONFIG = 8 };
	const char * aNamesConfig[NUM_FRAMES_CONFIG] = \
		{"AIN0_NEGATIVE_CH", "AIN0_RANGE", "AIN0_RESOLUTION_INDEX", "AIN0_SETTLING_US",
		 "AIN1_NEGATIVE_CH", "AIN1_RANGE", "AIN1_RESOLUTION_INDEX", "AIN1_SETTLING_US"};
	const double aValuesConfig[NUM_FRAMES_CONFIG] = {199, 10, 0, 0,
	                                                 199, 10, 0, 0};

	// Set up for reading AIN values
	enum { NUM_FRAMES_AIN = 2 };
	double aValuesAIN[NUM_FRAMES_AIN] = {0};
	const char * aNamesAIN[NUM_FRAMES_AIN] = {"AIN0", "AIN1"};

	int msDelay = 1000;

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

	printf("\nStarting read loop.  Press Ctrl+c to stop.\n");

	// Note: The LabJackM (LJM) library will catch the Ctrl+c signal, close
	//       all open devices, then exit the program.
	// TODO: Have this example program catch the Ctrl+c signal and exit more politely
	while (1) {
		// Read AIN from the LabJack
		err = LJM_eReadNames(handle, NUM_FRAMES_AIN, aNamesAIN, aValuesAIN,
			&errorAddress);
		ErrorCheckWithAddress(err, errorAddress, "LJM_eReadNames");

		// Print results
		printf("%s : %f V, %s : %f V\n", aNamesAIN[0], aValuesAIN[0],
			aNamesAIN[1], aValuesAIN[1]);

		MillisecondSleep(msDelay);
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
