/**
 * Name: single_dio_read.c
 * Desc: Demonstrates how to read a single digital input/output.
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

	// Set up for reading DIO state
	double Value = 0;
	const char * Name = "FIO0";

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Read DIO state from the LabJack
	err = LJM_eReadName(handle, Name, &Value);
	ErrorCheck(err, "LJM_eReadName");

	printf("\n%s state : %f\n", Name, Value);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
