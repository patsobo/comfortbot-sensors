/**
 * Name: stream_all_or_none.c
 * Desc: Shows how to stream with the T7 in external clock stream mode.
 * Note: Similar to also callback_stream.c, which uses a callback.
**/

#include <stdio.h>
#include <string.h>

#include "LabJackM.h"

#include "LJM_StreamUtilities.h"

#define FIO0_PULSE_OUT 1

#define SCAN_RATE 1000
const int SCANS_PER_READ = SCAN_RATE / 2;

enum { NUM_CHANNELS = 4 };
const char * POS_NAMES[] = {"AIN0",  "FIO_STATE",  "SYSTEM_TIMER_20HZ", "STREAM_DATA_CAPTURE_16"};

const int NUM_LOOP_ITERATIONS = 50;

void StreamReturnAllOrNone(int handle);

int main()
{
	int handle;

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	printf("\n");
	PrintDeviceInfoFromHandle(handle);
	GetAndPrint(handle, "FIRMWARE_VERSION");
	printf("\n");

	DisableStreamIfEnabled(handle);

	StreamReturnAllOrNone(handle);

	CloseOrDie(handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void StreamReturnAllOrNone(int handle)
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

	// Configure LJM for unpredictable stream timing
	SetConfigValue(LJM_STREAM_SCANS_RETURN, LJM_STREAM_SCANS_RETURN_ALL_OR_NONE);
	SetConfigValue(LJM_STREAM_RECEIVE_TIMEOUT_MODE, LJM_STREAM_RECEIVE_TIMEOUT_MODE_MANUAL);
	SetConfigValue(LJM_STREAM_RECEIVE_TIMEOUT_MS, 100);

	err = LJM_NamesToAddresses(NUM_CHANNELS, POS_NAMES, aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	SetupExternalClockStream(handle);

	// If you do not have a signal generator of some sort, you can connect a
	// wire from FIO0 to CIO3 and call EnableFIO0PulseOut to verify
	// that your program is working.
	if (FIO0_PULSE_OUT) {
		EnableFIO0PulseOut(handle, SCAN_RATE, SCAN_RATE * NUM_LOOP_ITERATIONS + 5000);
	}

	err = LJM_eStreamStart(handle, SCANS_PER_READ, NUM_CHANNELS, aScanList,
		&scanRate);
	ErrorCheck(err, "LJM_eStreamStart");

	while (streamRead++ < NUM_LOOP_ITERATIONS) {
		VariableStreamSleep(SCANS_PER_READ, SCAN_RATE, LJMScanBacklog);

		err = LJM_eStreamRead(handle, aData, &deviceScanBacklog, &LJMScanBacklog);
		if (err == LJME_NO_SCANS_RETURNED) {
			// printf("Stream has not collected %d scans yet.\n", SCANS_PER_READ);
			printf(".");
			fflush(stdout);
		}
		else {
			ErrorCheck(err, "LJM_eStreamRead");
			printf("\n");
			HardcodedPrintScans(POS_NAMES, aData, SCANS_PER_READ, NUM_CHANNELS,
				deviceScanBacklog, LJMScanBacklog);
		}
	}

	err = LJM_eStreamStop(handle);
	ErrorCheck(err, "Stopping stream");

	free(aData);
	free(aScanList);

	printf("\nDone with %d iterations\n", NUM_LOOP_ITERATIONS);
}
