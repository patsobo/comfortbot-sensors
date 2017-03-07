/**
 * Name: eReadName.c
 * Desc: Shows how to use the LJM_eReadName function
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include "LabJackM.h"

// For LabJackM helper functions
#include "LJM_Utilities.h"

int main()
{
	int err, handle;
	double value = 0;

	const char * NAME = "SERIAL_NUMBER";

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	err = LJM_eReadName(handle, NAME, &value);
	ErrorCheck(err, "LJM_eReadName");

	// Print results
	printf("%s: %f\n", NAME, value);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
