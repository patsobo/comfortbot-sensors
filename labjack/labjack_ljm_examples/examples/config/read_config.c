/**
 * Name: read_config.c
 * Desc: Demonstrates how to read configuration settings on a LabJack.
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

	// Set up read operation
	enum { NUM_FRAMES = 10};
	const char * aNames[NUM_FRAMES] = {"PRODUCT_ID", "HARDWARE_VERSION", "FIRMWARE_VERSION",
		"BOOTLOADER_VERSION", "WIFI_VERSION", "SERIAL_NUMBER", "POWER_ETHERNET_DEFAULT",
		"POWER_WIFI_DEFAULT", "POWER_AIN_DEFAULT", "POWER_LED_DEFAULT"};
	double aValues[NUM_FRAMES] = {0};
	int errorAddress = INITIAL_ERR_ADDRESS;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Read config values
	err = LJM_eReadNames(handle, NUM_FRAMES, aNames, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadNames");

	printf("\nConfiguration settings:\n");
	for (i=0; i<NUM_FRAMES; i++) {
		printf("    %s : %f\n", aNames[i], aValues[i]);
	}

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
