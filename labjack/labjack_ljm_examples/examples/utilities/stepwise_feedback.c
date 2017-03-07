/**
 * Name: stepwise_feedback.c
 * Desc: Shows how to read from a few analog inputs
 *       using a LabJack and the cross platform LabJackM
 *       Library using the C-style API
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LabJackM.h>

#include "../LJM_Utilities.h"

int main()
{
  int handle, err;

  // Just something that isn't positive so we know
  // errAddress has or hasn't been modified
  int errAddress = -2;

  // Set up the data
  enum { NUM_FRAMES = 2 };
  int numFrames = NUM_FRAMES;

  const char * ADDRESS_STRINGS[NUM_FRAMES] =
    {"DAC0", "AIN0"};
  const double VALUE_0 = 1.23;
  const double VALUE_1 = 0.0;

  int aAddresses[NUM_FRAMES];
  int     aTypes[NUM_FRAMES];
  int    aWrites[NUM_FRAMES] = {LJM_WRITE, LJM_READ };
  int aNumValues[NUM_FRAMES] = {1,         1        };

  enum { NUM_VALUES = 2 };
  double aValues[NUM_VALUES] = {VALUE_0,   VALUE_1  };

  int MaxBytesPerMBFB = LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE;
  unsigned char aMBFB[LJM_DEFAULT_FEEDBACK_ALLOCATION_SIZE];

  unsigned char UnitID = LJM_DEFAULT_UNIT_ID;

  // Fill out aAddresses
  err = LJM_NamesToAddresses(NUM_FRAMES, ADDRESS_STRINGS,
    aAddresses, aTypes);
  ErrorCheck(err, "LJM_NamesToAddresses");

  // Open first found LabJack
  err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
  ErrorCheck(err, "LJM_Open");

  PrintDeviceInfoFromHandle(handle);

  err = LJM_AddressesToMBFB(MaxBytesPerMBFB, aAddresses,
    aTypes, aWrites, aNumValues, aValues, &numFrames,
	aMBFB);
  ErrorCheck(err, "LJM_AddressesToMBFB");

  printf("\nLJM_MBFBComm will overwrite the Transaction ID and Unit ID of the following command\n");
  PrintFeedbackCommand(aMBFB, "Feedback command");

  // Send the command and receive the response
  err = LJM_MBFBComm(handle, UnitID, aMBFB, &errAddress);
  ErrorCheckWithAddress(err, errAddress, "LJM_MBFBComm");

  PrintFeedbackResponse(aMBFB, "Feedback response");

  // Get the data back in a readable form
  err = LJM_UpdateValues(aMBFB, aTypes, aWrites,
    aNumValues, numFrames, aValues);
  ErrorCheck(err, "LJM_UpdateValues");

  // Print results
  printf("%s: %f\n", ADDRESS_STRINGS[1], aValues[1]);

  // Close
  err = LJM_Close(handle);
  ErrorCheck(err, "LJM_Close");

  WaitForUserIfWindows();

  return LJME_NOERROR;
}
