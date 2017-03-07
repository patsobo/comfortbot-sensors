/**
 * Name: stream_example.c
 * Desc: Shows how to stream from a device
**/

#include <stdio.h>
#include <string.h>

#include "LabJackM.h"

#include "LJM_StreamUtilities.h"

void Stream(int handle, int numChannels, const char ** channelNames, double scanRate,
	int scansPerRead, int numSeconds);


/**
 * Global values to quickly configure some of this program's behavior
**/

// Output style
typedef enum {
	SCAN_OUTPUT_NONE,
	SCAN_OUTPUT_FINAL_READ,
	SCAN_OUTPUT_ALL_READS
} ScanReadPrintStyle;
const ScanReadPrintStyle READ_OUTPUT = SCAN_OUTPUT_ALL_READS;

// How long to stream for
const int NUM_SECONDS = 10;

// Channels/Addresses to stream. NUM_CHANNELS can be less than or equal to the
// size of POS_NAMES
enum { NUM_CHANNELS = 3 };
const char * POS_NAMES[] = {
	"AIN0",  "AIN1",  "FIO_STATE",  "AIN6",  "AIN8", "AIN10",
	"AIN12", "AIN14", "AIN16", "AIN18", "AIN20", "AIN22",
	"AIN24", "AIN26", "AIN28", "AIN30", "AIN32", "AIN34",
	"AIN36", "AIN38", "AIN40", "AIN42", "AIN44", "AIN46",
	"AIN48", "AIN50", "AIN52", "AIN54", "AIN56", "AIN58"};

int main()
{
	int handle;

	// How fast to stream in Hz
	double scanRate = 2000;

	// How many scans to get per call to LJM_eStreamRead. scanRate/2 is recommended
	int scansPerRead = scanRate / 2;

	// GetAndPrintConfigValue is defined in LJM_Utilities.h
	GetAndPrintConfigValue(LJM_LIBRARY_VERSION);
	GetAndPrintConfigValue(LJM_STREAM_TRANSFERS_PER_SECOND);

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	printf("\n");
	PrintDeviceInfoFromHandle(handle);
	GetAndPrint(handle, "FIRMWARE_VERSION");
	printf("\n");

	DisableStreamIfEnabled(handle);

	Stream(handle, NUM_CHANNELS, POS_NAMES, scanRate, scansPerRead, NUM_SECONDS);

	CloseOrDie(handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void Stream(int handle, int NUM_CHANNELS, const char ** POS_NAMES, double scanRate,
	int scansPerRead, int NUM_SECONDS)
{
	int err, i;
	int totalSkippedScans = 0;
	int deviceScanBacklog = 0;
	int LJMScanBacklog = 0;
	int streamBufferSize = 0;

	int * aScanList = malloc(sizeof(int) * NUM_CHANNELS);

	int NUM_READS = 0;
	unsigned int aDataSize = NUM_CHANNELS * scansPerRead;
	double * aData = malloc(sizeof(double) * aDataSize);
	unsigned int timeStart, timeEnd;

	const int LJM_WARNING_SCANS_NUM = scansPerRead * NUM_CHANNELS * 2;
	int deviceWarningScansNum = 10;
	double deviceBufferBytes = 0;

	err = LJM_NamesToAddresses(NUM_CHANNELS, POS_NAMES, aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	printf("channels:\n");
	for (i=0; i<NUM_CHANNELS; i++) {
		printf("    %s (%d)\n", POS_NAMES[i], aScanList[i]);
	}
	printf("\n");

	printf("Writing configurations:\n");

	printf("    Setting STREAM_BUFFER_SIZE_BYTES to %d\n", streamBufferSize);
	WriteNameOrDie(handle, "STREAM_BUFFER_SIZE_BYTES", streamBufferSize);

	printf("    Ensuring triggered stream is disabled. (Setting STREAM_TRIGGER_INDEX to 0)\n");
	WriteNameOrDie(handle, "STREAM_TRIGGER_INDEX", 0);

	printf("    Enabling internally-clocked stream. (Setting STREAM_CLOCK_SOURCE to 0)\n");
	WriteNameOrDie(handle, "STREAM_CLOCK_SOURCE", 0);

    // Configure the analog inputs' negative channel, range, settling time and
    // resolution.
    // Note when streaming, negative channels and ranges can be configured for
    // individual analog inputs, but the stream has only one settling time and
    // resolution.
	printf("    Setting STREAM_RESOLUTION_INDEX to 0\n");
	WriteNameOrDie(handle, "STREAM_RESOLUTION_INDEX", 0);

	printf("    Setting STREAM_SETTLING_US to 0\n");
	WriteNameOrDie(handle, "STREAM_SETTLING_US", 0);

	printf("    Setting AIN_ALL_RANGE to 0\n");
	WriteNameOrDie(handle, "AIN_ALL_RANGE", 0);

	printf("    Setting AIN_ALL_NEGATIVE_CH to LJM_GND\n");
	WriteNameOrDie(handle, "AIN_ALL_NEGATIVE_CH", LJM_GND);

	printf("\nReading the %d stream channels:\n", NUM_CHANNELS);
	for (i=0; i<NUM_CHANNELS; i++) {
		printf("    ");
		GetAndPrint(handle, POS_NAMES[i]);
	}

	printf("\n");

	printf("Starting stream:\n");
	printf("    scan rate: %.02f Hz (%.02f sample rate)\n",
		scanRate, scanRate * NUM_CHANNELS);
	printf("    scansPerRead: %d\n", scansPerRead);
	printf("    ");GetAndPrint(handle, "STREAM_RESOLUTION_INDEX");

	err = LJM_eStreamStart(handle, scansPerRead, NUM_CHANNELS, aScanList, &scanRate);
	ErrorCheck(err, "LJM_eStreamStart");

	NUM_READS = CalculateNumReads(NUM_SECONDS, scanRate, scansPerRead);
	printf("Stream started. Actual scanRate: %f\n", scanRate);
	printf("\n");

	err = LJM_eReadName(handle, "STREAM_BUFFER_SIZE_BYTES", &deviceBufferBytes);
	ErrorCheck(err, "Reading STREAM_BUFFER_SIZE_BYTES");
	deviceWarningScansNum = deviceBufferBytes / NUM_CHANNELS / 2 / 8; // 1/8th of the total scans
	printf("STREAM_BUFFER_SIZE_BYTES: %f, deviceWarningScansNum: %d\n", deviceBufferBytes,
		deviceWarningScansNum);
	printf("LJM_WARNING_SCANS_NUM: %d\n", LJM_WARNING_SCANS_NUM);

	// Read the scans
	printf("Now performing %d reads, expected to take approximately %d seconds\n",
		NUM_READS, NUM_SECONDS);
	timeStart = GetCurrentTimeMS();
	for (i=0; i<NUM_READS; i++) {
		printf("\n");

		err = LJM_eStreamRead(handle, aData, &deviceScanBacklog, &LJMScanBacklog);
		if (err != LJME_NOERROR) {
			printf("Error values:\n");
			PrintScans(scansPerRead, NUM_CHANNELS, POS_NAMES, aScanList,
				deviceScanBacklog, LJMScanBacklog, i, aData);
		}
		ErrorCheck(err, "LJM_eStreamRead");

		OutputStreamIterationInfo(i, deviceScanBacklog, deviceWarningScansNum,
			LJMScanBacklog, LJM_WARNING_SCANS_NUM);

		PrintScans(scansPerRead, NUM_CHANNELS, POS_NAMES, aScanList,
			deviceScanBacklog, LJMScanBacklog, i, aData);

		totalSkippedScans += CountAndOutputNumSkippedScans(NUM_CHANNELS, scansPerRead, aData);
	}
	timeEnd = GetCurrentTimeMS();
	PrintStreamConclusion(timeStart, timeEnd, i, scansPerRead, NUM_CHANNELS,
		totalSkippedScans);

	err = LJM_eStreamStop(handle);
	ErrorCheck(err, "Stopping stream");

	free(aData);
	free(aScanList);
}
