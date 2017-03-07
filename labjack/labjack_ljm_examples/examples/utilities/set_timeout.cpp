/**
 * Name: set_timeout.c
 * Desc: Shows how to set the timeout for command and response
**/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LabJackM.h"

#include "../LJM_Utilities.h"

void TestTimeout(const char * shortTimeoutDescription, const unsigned char * aMBFBCommand,
	int * aTypes, int * aWrites, int * aNumValues, double * aValues, int numFrames);

int main()
{
	int err;

	// Set up the data
	const int ADDRESS_0 = 1000;
	const double VALUE_0 = 1.23;

	const int ADDRESS_1 = 0;
	const double VALUE_1 = 0.0; // Doesn't really matter

	int aAddresses[] = {ADDRESS_0,   ADDRESS_1  };
	int     aTypes[] = {LJM_FLOAT32, LJM_FLOAT32};
	int    aWrites[] = {LJM_WRITE,   LJM_READ   };
	int aNumValues[] = {1,           1          };
	double aValues[] = {VALUE_0,     VALUE_1    };

	int numFrames = 2;
	int * NumFramesPtr = &numFrames;

	//const int ALLOCATION_SIZE = LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE; // The actual command size should be 20, this is just to simulate allocating the default amount
	int MaxBytesPerMBFB = LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE; // The default
	unsigned char aMBFB[LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE];

	// Set stdout to immediately print everything
	setbuf(stdout, NULL);

	// Create the Feedback command to write to DAC0 and read from AIN0
	err = LJM_AddressesToMBFB(MaxBytesPerMBFB, aAddresses, aTypes, aWrites, aNumValues, aValues, NumFramesPtr, aMBFB);
	ErrorCheck(err, "LJM_AddressesToMBFB");

	// Can now see/alter the contents of aMBFB which is a valid Feedback command
	PrintFeedbackCommand(aMBFB, "Feedback command");

	// Send the command and receive the response with different timeout lengths
	TestTimeout("Default", aMBFB, aTypes, aWrites, aNumValues, aValues, numFrames);

	err = LJM_WriteLibraryConfigS(LJM_SEND_RECEIVE_TIMEOUT_MS, 1);
	ErrorCheck(err, "Setting send/receive timeout to 1 ms");
	TestTimeout("1ms", aMBFB, aTypes, aWrites, aNumValues, aValues, numFrames);

	err = LJM_WriteLibraryConfigS(LJM_SEND_RECEIVE_TIMEOUT_MS, 0);
	ErrorCheck(err, "Setting send/receive timeout to no timeout");
	TestTimeout("Never-timeout", aMBFB, aTypes, aWrites, aNumValues, aValues, numFrames);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void TestTimeout(const char * shortTimeoutDescription, const unsigned char * aMBFBCommand,
	int * aTypes, int * aWrites, int * aNumValues, double * aValues, int numFrames)
{
	int err;
	int errAddress = -2; // Just something that isn't positive so we know errAddress has or hasn't been modified
	unsigned char UnitID = 0; // Or something
	int handle;

	unsigned char aMBFBTemp[LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE];

	// Copy the command to a temp array, so that the temp array is updated with the response
	// but the command remains the same
	memcpy(aMBFBTemp, aMBFBCommand, LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE);

	printf("LJM_MBFBComm with %s timeout... ", shortTimeoutDescription);

	// Execute the command
	err = LJM_OpenS("LJM_dtT7", "LJM_ctTCP", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_OpenS");

	err = LJM_MBFBComm(handle, UnitID, aMBFBTemp, &errAddress);
	ErrorCheck(LJM_Close(handle), "LJM_Close");

	// Check if it timed out or not
	if (err == LJME_NO_COMMAND_BYTES_SENT)
		printf("The error LJME_NO_COMMAND_BYTES_SENT occured, which indicates that the command timed out.\n");
	else if (err == LJME_NO_RESPONSE_BYTES_RECEIVED)
		printf("The error LJME_NO_RESPONSE_BYTES_RECEIVED occured, which indicates that the response timed out.\n");
	else if (err == LJME_INCORRECT_NUM_RESPONSE_BYTES_RECEIVED)
		printf("The error LJME_INCORRECT_NUM_RESPONSE_BYTES_RECEIVED occured, which indicates that the response timed out.\n");
	else if (err != LJME_NOERROR) {
		printf("An error occured that wasn't a timeout error: ");
		ErrorCheckWithAddress(err, errAddress, "LJM_MBFB");
	}
	else {
		printf("success! No timeout.\n");
		PrintFeedbackResponse(aMBFBTemp, "Here's the response bytes");

		// Get the data back in a readable form
		err = LJM_UpdateValues(aMBFBTemp, aTypes, aWrites, aNumValues, numFrames, aValues);
		ErrorCheck(err, "LJM_UpdateValues");

		// Print results
		printf("And here's the results:\nAIN 0: %f\n", aValues[1]);
	}

	printf("\n\n");
}
