/**
 * Name: stream_triggered.c
 * Desc: Shows how to stream with the T7 using triggered stream on DIO0 / FIO0.
**/

#include <stdio.h>
#include <string.h>

#include "LabJackM.h"

#include "LJM_StreamUtilities.h"

#define SCAN_RATE 1000
const int SCANS_PER_READ = SCAN_RATE / 2;

enum { NUM_CHANNELS = 4 };
const char * POS_NAMES[] = {"AIN0",  "FIO_STATE",  "SYSTEM_TIMER_20HZ", "STREAM_DATA_CAPTURE_16"};

const int NUM_LOOP_ITERATIONS = 10;

void StreamTriggered(int handle);

int main()
{
	int handle;

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");

	PrintDeviceInfoFromHandle(handle);
	GetAndPrint(handle, "FIRMWARE_VERSION");
	printf("\n");

	DisableStreamIfEnabled(handle);

	StreamTriggered(handle);

	CloseOrDie(handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void StreamTriggered(int handle)
{
	int err;

	// Variables for LJM_eStreamStart
	double scanRate = SCAN_RATE;
	int * aScanList = malloc(sizeof(int) * NUM_CHANNELS);

	// Variables for LJM_eStreamRead
	unsigned int aDataSize = NUM_CHANNELS * SCANS_PER_READ;
	double * aData = malloc(sizeof(double) * aDataSize);
	int deviceScanBacklog = 0;
	int LJMScanBacklog = 0;
	int streamRead = 0;

	err = LJM_NamesToAddresses(NUM_CHANNELS, POS_NAMES, aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	// Configure LJM for unpredictable stream timing
	SetConfigValue(LJM_STREAM_SCANS_RETURN, LJM_STREAM_SCANS_RETURN_ALL_OR_NONE);
	SetConfigValue(LJM_STREAM_RECEIVE_TIMEOUT_MS, 0);

	// 2000 sets DIO0 / FIO0 as the stream trigger
	WriteNameOrDie(handle, "STREAM_TRIGGER_INDEX", 2000);

	// Clear any previous DIO0_EF settings
	WriteNameOrDie(handle, "DIO0_EF_ENABLE", 0);

	// 5 enables a rising or falling edge to trigger stream
	WriteNameOrDie(handle, "DIO0_EF_INDEX", 5);

	// Enable DIO0_EF
	WriteNameOrDie(handle, "DIO0_EF_ENABLE", 1);

	err = LJM_eStreamStart(handle, SCANS_PER_READ, NUM_CHANNELS, aScanList,
		&scanRate);
	ErrorCheck(err, "LJM_eStreamStart");

	printf("You can trigger stream now via a rising or falling edge on DIO0 / FIO0.\n");
	printf("(Press ctrl + c to cancel.)\n");

	while (streamRead < NUM_LOOP_ITERATIONS) {
		VariableStreamSleep(SCANS_PER_READ, SCAN_RATE, LJMScanBacklog);

		err = LJM_eStreamRead(handle, aData, &deviceScanBacklog, &LJMScanBacklog);
		if (err == LJME_NO_SCANS_RETURNED) {
			printf(".");
			fflush(stdout);
		}
		else {
			ErrorCheck(err, "LJM_eStreamRead");
			printf("\n");
			HardcodedPrintScans(POS_NAMES, aData, SCANS_PER_READ, NUM_CHANNELS,
				deviceScanBacklog, LJMScanBacklog);
			++streamRead;
		}
	}

	err = LJM_eStreamStop(handle);
	ErrorCheck(err, "Stopping stream");

	free(aData);
	free(aScanList);

	printf("\nDone with %d iterations\n", NUM_LOOP_ITERATIONS);
}
