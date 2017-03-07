/**
 * Name: write_device_name_string.c
 * Desc: Demonstrates how to write the device name string to a LabJack.
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
	const char * NAME_TO_WRITE = "My Awesome LabJack Device";

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Write
	printf("\nWriting \"%s\" to %s\n", NAME_TO_WRITE, NAME_REGISTER);
	err = LJM_eWriteNameString(handle, NAME_REGISTER, NAME_TO_WRITE);
	ErrorCheck(err, "LJM_eWriteNameString");

	// The LJM_eWriteAddressString version
	// int Address = LJM_INVALID_NAME_ADDRESS;
	// int Type = LJM_INVALID_NAME_ADDRESS;
	// err = LJM_NameToAddress(NAME_REGISTER, &Address, &Type);
	// ErrorCheck(err, "LJM_NameToAddress");
	// err = LJM_eWriteAddressString(handle, Address, NAME_TO_WRITE);
	// ErrorCheck(err, "LJM_eWriteAddressString");

	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	printf("\nThe device may need to be power-cycled before the name change can be read.\n");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
