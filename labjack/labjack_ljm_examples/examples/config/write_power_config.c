/**
 * Name: write_power_config.c
 * Desc: Demonstrates how to configure default power settings on a LabJack.
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

	// Set up operation
	enum { NUM_FRAMES = 4};
	const char * aNames[NUM_FRAMES] = {"POWER_ETHERNET_DEFAULT", "POWER_WIFI_DEFAULT",
		"POWER_AIN_DEFAULT", "POWER_LED_DEFAULT"};
	double aValues[NUM_FRAMES] = {1, 0, 1, 1};
	int errorAddress = INITIAL_ERR_ADDRESS;

	err = LJM_Open(LJM_dtANY, LJM_ctUSB, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Perform operation
	err = LJM_eWriteNames(handle, NUM_FRAMES, aNames, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eWriteNames");

	printf("\nConfigurations set:\n");
	for (i=0; i<NUM_FRAMES; i++) {
		printf("    %s : %.0f\n", aNames[i], aValues[i]);
	}

	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
