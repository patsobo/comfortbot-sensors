/**
 * Name: read_device_name_string.c
 * Desc: Demonstrates how to read the device name string from a LabJack.
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

	const char * NAME_REGISTER = "DEVICE_NAME_DEFAULT";
	char allocatedString[LJM_STRING_ALLOCATION_SIZE];

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Initial read
	err = LJM_eReadNameString(handle, NAME_REGISTER, allocatedString);
	ErrorCheck(err, "LJM_eReadNameString");

	// The LJM_eReadAddressString version
	// int Address = LJM_INVALID_NAME_ADDRESS;
	// int Type = LJM_INVALID_NAME_ADDRESS;
	// err = LJM_NameToAddress(NAME_REGISTER, &Address, &Type);
	// ErrorCheck(err, "LJM_NameToAddress");
	// err = LJM_eReadAddressString(handle, Address, allocatedString);
	// ErrorCheck(err, "LJM_eReadAddressString");
	// printf("\n%s : %s\n", NAME_REGISTER, allocatedString);

	printf("\n%s : %s\n", NAME_REGISTER, allocatedString);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
