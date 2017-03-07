/**
 * Name: stream_burst.c
 * Desc: Shows how to stream from a device and stop after a given number of
 *       scans.
**/

#include <stdio.h>
#include <string.h>

#include "LabJackM.h"

#include "LJM_StreamUtilities.h"

void StreamBurst(int handle, int numChannels, const char ** channelNames,
	double scanRate, int numScans);


/**
 * Global values to quickly configure some of this program's behavior
**/

enum { NUM_CHANNELS = 2 };
const char * POS_NAMES[] = {"AIN0",  "FIO_STATE"};


int main(int argc, char * argv[])
{
	int handle;
	int numScans = 0;

	// How fast to stream in Hz.
	double scanRate = 2000;

	if (argc != 2) {
		printf("Usage: %s STREAM_NUM_SCANS\n", argv[0]);
		WaitForUserIfWindows();
		exit(1);
	}

	numScans = strtol(argv[1], NULL, 10);
	if (numScans <= 0) {
		printf("STREAM_NUM_SCANS must be greater than 0\n");
		printf("Usage: %s STREAM_NUM_SCANS\n", argv[0]);
		WaitForUserIfWindows();
		exit(1);
	}

	// GetAndPrintConfigValue is defined in LJM_Utilities.h
	GetAndPrintConfigValue(LJM_LIBRARY_VERSION);
	GetAndPrintConfigValue(LJM_STREAM_TRANSFERS_PER_SECOND);

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	printf("\n");
	PrintDeviceInfoFromHandle(handle);
	GetAndPrint(handle, "FIRMWARE_VERSION");
	printf("\n");

	DisableStreamIfEnabled(handle);

	StreamBurst(handle, NUM_CHANNELS, POS_NAMES, scanRate, numScans);

	CloseOrDie(handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void StreamBurst(int handle, int numChannels, const char ** channelNames,
	double scanRate, int numScans)
{
	int err, chanIter;

	int * aScanList = malloc(sizeof(int) * numChannels);

	unsigned int numSamples = numChannels * numScans;
	double * aBurstSamples = malloc(sizeof(double) * numSamples);

	unsigned int timeStart, timeEnd;

	memset(aBurstSamples, 0, numSamples * sizeof(double));

	err = LJM_NamesToAddresses(numChannels, POS_NAMES, aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	printf("channels:\n");
	for (chanIter = 0; chanIter < numChannels; chanIter++) {
		printf("    %s (%d)\n", POS_NAMES[chanIter], aScanList[chanIter]);
	}
	printf("\n");

	printf("Writing configurations:\n");

	printf("    Ensuring triggered stream is disabled. (Setting STREAM_TRIGGER_INDEX to 0)\n");
	WriteNameOrDie(handle, "STREAM_TRIGGER_INDEX", 0);

	printf("    Enabling internally-clocked stream. (Setting STREAM_CLOCK_SOURCE to 0)\n");
	WriteNameOrDie(handle, "STREAM_CLOCK_SOURCE", 0);

	printf("\n");

	printf("Starting stream:\n");
	printf("    scan rate: %.02f Hz (%.02f sample rate)\n",
		scanRate, scanRate * numChannels);
	printf("    STREAM_NUM_SCANS: %u\n", numScans);
	printf("    ");GetAndPrint(handle, "STREAM_RESOLUTION_INDEX");

	timeStart = GetCurrentTimeMS();
	err = LJM_StreamBurst(handle, numChannels, aScanList, &scanRate, numScans,
		aBurstSamples);
	timeEnd = GetCurrentTimeMS();
	ErrorCheck(err, "LJM_eStreamBurst");

	printf("\n");
	printf("Stream burst complete:\n");
	printf("    Actual scanRate was: %f\n", scanRate);
	printf("    %d scans over approximately %d milliseconds\n",
		numScans, timeEnd - timeStart);
	printf("\n");

	PrintScans(numScans, NUM_CHANNELS, POS_NAMES, aScanList, 0, 0, 0,
		aBurstSamples);

	CountAndOutputNumSkippedScans(NUM_CHANNELS, numScans, aBurstSamples);

	free(aBurstSamples);
	free(aScanList);
}
