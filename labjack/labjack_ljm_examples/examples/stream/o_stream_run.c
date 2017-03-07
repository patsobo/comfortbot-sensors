/**
 * Name: o_stream_run.c
 * Desc: Sets up stream-in and stream-out together, then reads stream-in values
 *       while updating stream-out with loop data.
 * Note: You can connect a wire from AIN0 to DAC0 and from AIN1 to DAC1 to see
 *       the effect of stream-out on stream-in.
**/

#include <stdio.h>
#include <string.h>

#include <LabJackM.h>

#include "LJM_StreamUtilities.h"


/**
 * Desc: Structure to encapsulate a single stream out channel.
 *       targetName, the stream out target (e.g. "DAC0")
 *       targetLoopSize, the number of target loop values
 *       targetLoopValues, an array of stream out values. Must be of size
 *           targetLoopSize or greater
**/
typedef struct StreamOutChannel {
	const char * targetName;
	const int targetLoopSize;
	const double * targetLoopValues;
} StreamOutChannel;

/**
 * Desc: Structure to encapsulate multiple stream out channels.
 *       numTargets, the number of stream out targets
**/
typedef struct StreamOutInfo {
	const int numTargets;
	const StreamOutChannel * streamOutChannels;
} StreamOutInfo;


/**
 * Global values to quickly configure some of this program's behavior
**/

// How long to stream for
const int NUM_SECONDS = 10;

// Channels/Addresses to stream in. NUM_IN_CHANNELS can be less than or equal to the
// size of POS_NAMES
enum { NUM_IN_CHANNELS = 2 };
const char * POS_IN_NAMES[] = {"AIN0", "AIN1"};
enum { NUM_OUT_CHANNELS = 2 };

// Set up the values for stream out
enum { STREAM_OUT0_LOOP_SIZE = 2 };
enum { STREAM_OUT1_LOOP_SIZE = 4 };
const double STREAM_OUT0_LOOP_VALUES [STREAM_OUT0_LOOP_SIZE] = {
	0.1, 1.0,
};
const double STREAM_OUT1_LOOP_VALUES [STREAM_OUT1_LOOP_SIZE] = {
	0.1, 1.0,
	2.0, 3.0
};
#define SO_CHANNEL0_CONTENT { \
	"DAC0", \
	STREAM_OUT0_LOOP_SIZE, \
	STREAM_OUT0_LOOP_VALUES, \
}
#define SO_CHANNEL1_CONTENT { \
	"DAC1", \
	STREAM_OUT1_LOOP_SIZE, \
	STREAM_OUT1_LOOP_VALUES, \
}
const StreamOutChannel SO_CHANNEL0 = SO_CHANNEL0_CONTENT;
const StreamOutChannel SO_CHANNEL1 = SO_CHANNEL1_CONTENT;


/**
 * Desc: Initializes / fills an array with stream channel addresses.
 * Para: NUM_IN_CHANNELS, the number of stream-in channels
 *       POS_NAMES, the stream-in channel names
 *       NUM_OUT_CHANNELS, the number of stream-out channels
 *       aScanList, the output list of stream channel addresses. aScanList is a
 *           pointer to an array of addresses
**/
void InitializeScanList(const int NUM_IN_CHANNELS, const char ** POS_NAMES,
	const int NUM_OUT_CHANNELS, int ** aScanList);

/**
 * Desc: Sets up stream out and streams in data while continuously updating
 *       stream out.
**/
void StreamOutAndIn(int handle, int numInChannels, const char ** channelNames,
	StreamOutInfo streamOutInfo, double scanRate, int scansPerRead,
	int numSeconds);

/**
 * Desc: Configures the device to do one or more stream out channels.
**/
void SetUpStreamOut(int handle, StreamOutInfo streamOutInfo);

/**
 * Desc: Set up a single stream out channel.
**/
void SetUpStreamOutTarget(int handle, int soOffset,
	StreamOutChannel soChannel);

/**
 * Desc: Updates all stream channels, multiplying all stream out values by a
 *       scalar.
**/
void UpdateStreamOutChannels(int handle, StreamOutInfo streamOutInfo,
	double scalar);

/**
 * Desc: Updates a single stream out channel, multiplying all stream out
 *       values by a scalar.
**/
void UpdateStreamOutChannel(int handle, int soOffset, StreamOutChannel streamOut,
	double scalar);


int main()
{
	int handle;

	// How fast to stream in Hz
	double scanRate = MAX_SCANS_TO_PRINT * 2;
	// Limiting scanRate to (MAX_SCANS_TO_PRINT * 2) will allow us to print
	// every scan, assuming scansPerRead is (scanRate / 2).

	// How many scans to get per call to LJM_eStreamRead.
	int scansPerRead = MAX_SCANS_TO_PRINT;

	const StreamOutChannel STREAM_OUT_CHANNELS[NUM_OUT_CHANNELS] = {
		SO_CHANNEL0_CONTENT,
		SO_CHANNEL1_CONTENT
	};
	const StreamOutInfo streamOutInfo = {
		NUM_OUT_CHANNELS,
		STREAM_OUT_CHANNELS
	};

	GetAndPrintConfigValue(LJM_LIBRARY_VERSION);
	GetAndPrintConfigValue(LJM_STREAM_TRANSFERS_PER_SECOND);

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	printf("\n");
	PrintDeviceInfoFromHandle(handle);
	GetAndPrint(handle, "FIRMWARE_VERSION");
	printf("\n");

	DisableStreamIfEnabled(handle);
	StreamOutAndIn(handle, NUM_IN_CHANNELS, POS_IN_NAMES, streamOutInfo,
		scanRate, scansPerRead, NUM_SECONDS);

	CloseOrDie(handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void StreamOutAndIn(int handle, const int NUM_IN_CHANNELS, const char ** POS_NAMES,
	StreamOutInfo streamOutInfo, double scanRate, int scansPerRead, int NUM_SECONDS)
{
	int err, i;
	int totalSkippedScans = 0;
	int deviceScanBacklog = 0;
	int LJMScanBacklog = 0;
	double sequenceOffset, scalar;

	const int TOTAL_NUM_CHANNELS = NUM_IN_CHANNELS + streamOutInfo.numTargets;

	int * aScanList = malloc(sizeof(int) * TOTAL_NUM_CHANNELS);

	int NUM_READS = CalculateNumReads(NUM_SECONDS, scanRate, scansPerRead);
	unsigned int aDataSize = NUM_IN_CHANNELS * scansPerRead;
	double * aData = malloc(sizeof(double) * aDataSize);
	unsigned int timeStart, timeEnd;

	const int LJM_WARNING_SCANS_NUM = scansPerRead * NUM_IN_CHANNELS * 2;
	int deviceWarningScansNum = 10;
	double deviceBufferBytes = 0;

	SetUpStreamOut(handle, streamOutInfo);

	// Fill aScanList with the names of the channels we are streaming
	InitializeScanList(NUM_IN_CHANNELS, POS_NAMES, streamOutInfo.numTargets,
		&aScanList);

	printf("\n");
	printf("Writing configurations:\n");

	printf("    Ensuring triggered stream is disabled. (Setting STREAM_TRIGGER_INDEX to 0)\n");
	WriteNameOrDie(handle, "STREAM_TRIGGER_INDEX", 0);

	printf("    Enabling internally-clocked stream. (Setting STREAM_CLOCK_SOURCE to 0)\n");
	WriteNameOrDie(handle, "STREAM_CLOCK_SOURCE", 0);

	printf("\n");

	err = LJM_eStreamStart(handle, scansPerRead, TOTAL_NUM_CHANNELS, aScanList, &scanRate);
	ErrorCheck(err, "LJM_eStreamStart");

	printf("Started stream:\n");
	printf("    actual scan rate: %.02f Hz\n", scanRate);
	printf("    scansPerRead: %d\n", scansPerRead);
	printf("    ");GetAndPrint(handle, "STREAM_RESOLUTION_INDEX");
	printf("\n");

	err = LJM_eReadName(handle, "STREAM_BUFFER_SIZE_BYTES", &deviceBufferBytes);
	ErrorCheck(err, "Reading STREAM_BUFFER_SIZE_BYTES");
	deviceWarningScansNum = deviceBufferBytes / NUM_IN_CHANNELS / 2 / 8; // 1/8th of the total scans
	printf("STREAM_BUFFER_SIZE_BYTES: %f, deviceWarningScansNum: %d\n", deviceBufferBytes,
		deviceWarningScansNum);
	printf("LJM_WARNING_SCANS_NUM: %d\n", LJM_WARNING_SCANS_NUM);

	// Read the scans
	printf("Now performing %d reads, expected to take approximately %d seconds\n",
		NUM_READS, NUM_SECONDS);
	timeStart = GetCurrentTimeMS();
	for (i = 0; i < NUM_READS; i++) {
		printf("\n");

		err = LJM_eStreamRead(handle, aData, &deviceScanBacklog, &LJMScanBacklog);
		if (err != LJME_NOERROR) {
			printf("Error values:\n");
			PrintScans(scansPerRead, NUM_IN_CHANNELS, POS_NAMES, aScanList,
				deviceScanBacklog, LJMScanBacklog, i, aData);
		}
		ErrorCheck(err, "LJM_eStreamRead");

		OutputStreamIterationInfo(i, deviceScanBacklog, deviceWarningScansNum,
			LJMScanBacklog, LJM_WARNING_SCANS_NUM);

		PrintScans(scansPerRead, NUM_IN_CHANNELS, POS_NAMES, aScanList,
			deviceScanBacklog, LJMScanBacklog, i, aData);

		// (i % 10) gives us [0, 1, ..., 8, 9]
		sequenceOffset = (i % 10);
		// The point is to get a value in [0.1, 0.2, ..., 0.9, 1.0]
		scalar = 1.0 - .1 * sequenceOffset;
		printf("Updating stream out with scalar: %f\n", scalar);
		UpdateStreamOutChannels(handle, streamOutInfo, scalar);

		totalSkippedScans += CountAndOutputNumSkippedScans(NUM_IN_CHANNELS, scansPerRead, aData);
	}
	timeEnd = GetCurrentTimeMS();

	PrintStreamConclusion(timeStart, timeEnd, NUM_READS, scansPerRead,
		NUM_IN_CHANNELS, totalSkippedScans);

	err = LJM_eStreamStop(handle);
	ErrorCheck(err, "Stopping stream");

	free(aData);
	free(aScanList);
}

void SetUpStreamOut(int handle, StreamOutInfo streamOutInfo)
{
	int i;
	for (i = 0; i < streamOutInfo.numTargets; i++) {
		SetUpStreamOutTarget(handle, i, streamOutInfo.streamOutChannels[i]);
	}
}

void PrintAndWriteNameOrDie(int handle, const char * name, double value)
{
	printf("Write: %s = %f\n", name, value);
	WriteNameOrDie(handle, name, value);
}

void SetUpStreamOutTarget(int handle, int soOffset, StreamOutChannel soChannel)
{
	int targetAddress = GetAddressFromNameOrDie(soChannel.targetName);

	// Set up the names
	char STREAM_OUT_TARGET[50];
	char STREAM_OUT_BUFFER_SIZE[50];
	char STREAM_OUT_ENABLE[50];
	char STREAM_OUT_BUFFER_STATUS[50];
	sprintf(STREAM_OUT_TARGET, "STREAM_OUT%d_TARGET", soOffset);
	sprintf(STREAM_OUT_BUFFER_SIZE, "STREAM_OUT%d_BUFFER_SIZE", soOffset);
	sprintf(STREAM_OUT_ENABLE, "STREAM_OUT%d_ENABLE", soOffset);
	sprintf(STREAM_OUT_BUFFER_STATUS, "STREAM_OUT%d_BUFFER_STATUS", soOffset);

	// Allocate memory for the stream-out buffer
	PrintAndWriteNameOrDie(handle, STREAM_OUT_TARGET, targetAddress);
	PrintAndWriteNameOrDie(handle, STREAM_OUT_BUFFER_SIZE, 512);
	PrintAndWriteNameOrDie(handle, STREAM_OUT_ENABLE, 1);

	UpdateStreamOutChannel(handle, soOffset, soChannel, 1);

	GetAndPrint(handle, STREAM_OUT_BUFFER_STATUS);
}

void UpdateStreamOutChannels(int handle, StreamOutInfo streamOutInfo,
	double scalar)
{
	int i;
	for (i = 0; i < streamOutInfo.numTargets; i++) {
		UpdateStreamOutChannel(handle, i, streamOutInfo.streamOutChannels[i],
			scalar);
	}
}

void UpdateStreamOutChannel(int handle, int soOffset,
	StreamOutChannel soChannel, double scalar)
{
	int valueI;
	double * aValues;

	// Set up the names
	char STREAM_OUT_LOOP_SIZE[50];
	char STREAM_OUT_BUFFER_F32[50];
	char STREAM_OUT_SET_LOOP[50];
	sprintf(STREAM_OUT_LOOP_SIZE, "STREAM_OUT%d_LOOP_SIZE", soOffset);
	sprintf(STREAM_OUT_BUFFER_F32, "STREAM_OUT%d_BUFFER_F32", soOffset);
	sprintf(STREAM_OUT_SET_LOOP, "STREAM_OUT%d_SET_LOOP", soOffset);

	aValues = malloc(sizeof(double) * soChannel.targetLoopSize);
	for (valueI = 0; valueI < soChannel.targetLoopSize; valueI++) {
		aValues[valueI] = scalar * soChannel.targetLoopValues[valueI];
	}

	WriteNameOrDie(handle, STREAM_OUT_LOOP_SIZE, soChannel.targetLoopSize);
	WriteNameArrayOrDie(handle, STREAM_OUT_BUFFER_F32, soChannel.targetLoopSize,
	    aValues);

	WriteNameOrDie(handle, STREAM_OUT_SET_LOOP, 1);
	free(aValues);
}

void InitializeScanList(const int NUM_IN_CHANNELS, const char ** POS_NAMES,
	const int NUM_OUT_CHANNELS, int ** aScanList)
{
	int i, err;

	// Fill aScanList with stream-in addresses
	err = LJM_NamesToAddresses(NUM_IN_CHANNELS, POS_NAMES, *aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	for (i = 0; i < NUM_OUT_CHANNELS; i++) {
		char STREAM_OUT_NAME[50];
		sprintf(STREAM_OUT_NAME, "STREAM_OUT%d", i);

		// aScanList[0] up to aScanList[NUM_IN_CHANNELS - 1] is filled with
		// stream in channels, so here we fill aScanList[NUM_IN_CHANNELS] up
		// to aScanList[TOTAL_NUM_CHANNELS - 1]
		(*aScanList)[NUM_IN_CHANNELS + i] = GetAddressFromNameOrDie(STREAM_OUT_NAME);
	}
}
