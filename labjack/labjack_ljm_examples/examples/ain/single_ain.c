/**
 * Name: single_ain.c
 * Desc: Demonstrates reading a single analog input (AIN) from a LabJack.
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

	// Set up for reading AIN value
	double Value = 0;
	const char * Name = "AIN0";

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Read AIN from the LabJack
	err = LJM_eReadName(handle, Name, &Value);
	ErrorCheck(err, "LJM_eReadName");

	// Print results
	printf("\n%s: %f V\n", Name, Value);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
