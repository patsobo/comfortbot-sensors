/**
 * Name: callback_stream.c
 * Desc: Shows how to stream using a callback to read stream, which is useful
 *       for streaming with the T7 in external clock stream mode.
**/

#include <stdio.h>
#include <string.h>

#include "LabJackM.h"

#include "LJM_StreamUtilities.h"

#define FALSE 0
#define TRUE 1

// Set to non-zero for external stream clock
#define EXTERNAL_STREAM_CLOCK FALSE

// Set FIO to pulse out. See EnableFIO0PulseOut()
#define FIO0_PULSE_OUT FALSE

typedef struct StreamInfo {
	int handle;
	double scanRate;
	int scansPerRead;

	int streamLengthMS;
	int done;

	LJM_StreamReadCallback callback;

	int numChannels;
	int * aScanList;
	const char ** channelNames;

	int aDataSize;
	double * aData;
} StreamInfo;

/**
 * The stream callback that LJM will call when stream data is ready.
**/
void MyStreamReadCallback(void * arg);

/**
 * Sets up stream using MyStreamReadCallback as the stream callback that LJM
 * will call when stream data is ready.
**/
void StreamWithCallback(StreamInfo * si);

int main()
{
	const char * CHANNEL_NAMES[] = \
		{"AIN0",  "FIO_STATE",  "SYSTEM_TIMER_20HZ", "STREAM_DATA_CAPTURE_16"};

	StreamInfo si;
	si.scanRate = 2000; // Set the scan rate to the fastest rate expected
	si.numChannels = 4;
	si.channelNames = CHANNEL_NAMES;
	si.scansPerRead = si.scanRate / 2;

	si.streamLengthMS = 10000;
	si.done = FALSE;

	si.callback = &MyStreamReadCallback;
	si.aData = NULL;
	si.aScanList = NULL;

	si.handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	PrintDeviceInfoFromHandle(si.handle);

	DisableStreamIfEnabled(si.handle);
	StreamWithCallback(&si);

	CloseOrDie(si.handle);

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void StreamWithCallback(StreamInfo * si)
{
	int err;

	// Timer
	unsigned int t0, t1;

	// Variables for LJM_eStreamStart
	si->aScanList = malloc(sizeof(int) * si->numChannels);
	si->aDataSize = si->numChannels * si->scansPerRead;
	si->aData = malloc(sizeof(double) * si->aDataSize);

	err = LJM_NamesToAddresses(si->numChannels, si->channelNames, si->aScanList, NULL);
	ErrorCheck(err, "Getting positive channel addresses");

	if (EXTERNAL_STREAM_CLOCK) {
		SetupExternalClockStream(si->handle);
	}

	// If you do not have a signal generator of some sort, you can connect a
	// wire from FIO0 to CIO3 and call EnableFIO0PulseOut to verify
	// that your program is working.
	if (FIO0_PULSE_OUT) {
		EnableFIO0PulseOut(si->handle, si->scanRate,
			si->scanRate * si->streamLengthMS / 1000 + 5000);
	}

	t0 = GetCurrentTimeMS();
	err = LJM_eStreamStart(si->handle, si->scansPerRead, si->numChannels, si->aScanList,
		&(si->scanRate));
	ErrorCheck(err, "LJM_eStreamStart");

	err = LJM_SetStreamCallback(si->handle, si->callback, si);
	ErrorCheck(err, "LJM_SetStreamCallback");

	printf("Stream running, callback set, sleeping for %d milliseconds\n", si->streamLengthMS);
	MillisecondSleep(si->streamLengthMS);

	printf("Stopping stream...\n");
	si->done = TRUE;
	err = LJM_eStreamStop(si->handle);
	t1 = GetCurrentTimeMS();
	ErrorCheck(err, "LJM_eStreamStop");

	printf("Stream stopped. %u milliseconds have elapsed since LJM_eStreamStart\n", t1 - t0);

	free(si->aScanList);
	free(si->aData);
}

void MyStreamReadCallback(void * arg)
{
	StreamInfo * si = arg;
	static int streamRead = 0;
	int deviceScanBacklog = 0;
	int LJMScanBacklog = 0;
	int err;

	// Check if stream is done so that we don't output the printf below
	if (si->done) {
		return;
	}
	printf("% 3d.\n", streamRead++);

	err = LJM_eStreamRead(si->handle, si->aData, &deviceScanBacklog, &LJMScanBacklog);
	printf("    ");
	HardcodedPrintScans(si->channelNames, si->aData, si->scansPerRead, si->numChannels,
		deviceScanBacklog, LJMScanBacklog);

	// If LJM has called this callback, the data is valid, but LJM_eStreamRead
	// may return LJME_STREAM_NOT_RUNNING if another thread has stopped stream,
	// such as this example program does in StreamWithCallback().
	if (err != LJME_STREAM_NOT_RUNNING) {
		PrintErrorIfError(err, "LJM_eStreamRead");
	}
}
