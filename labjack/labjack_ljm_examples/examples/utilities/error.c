/**
 * Name: error.c
 * Desc: Demonstrates LJM_ErrorToString
**/

#include <stdio.h>
#include "LabJackM.h"

#include "../LJM_Utilities.h"

void manualErrorDisplay(int err)
{
	char errName[LJM_MAX_NAME_SIZE];
	LJM_ErrorToString(err, errName);
	printf("LJM_ErrorToString(%d) returned %s\n", err, errName);
}

int main()
{
	printf("Manual values:\n");
	manualErrorDisplay(0);
	manualErrorDisplay(LJME_CONSTANTS_FILE_NOT_FOUND);
	manualErrorDisplay(LJME_INVALID_CONSTANTS_FILE);
	manualErrorDisplay(LJME_TRANSACTION_ID_ERR);
	manualErrorDisplay(LJME_WARNINGS_BEGIN);
	manualErrorDisplay(LJME_U3_CANNOT_BE_OPENED_BY_LJM);
	manualErrorDisplay(199);
	manualErrorDisplay(2330); // T7 error

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
