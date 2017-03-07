/**
 * Name: LJM_StreamUtilities.c
 * Desc: Provides some basic helper functions for stream applications
**/

#ifndef LJM_STREAM_UTILITIES
#define LJM_STREAM_UTILITIES


#include "../LJM_Utilities.h"

// Limit how many scans should be printed for each call to PrintScans
enum { MAX_SCANS_TO_PRINT = 4 };


// STREAM_OUT#(0:3)_LOOP_SIZE
enum { SET_LOOP_USE_NEW_DATA_IMMEDIATELY = 1 };
enum { SET_LOOP_WAIT_FOR_SYNCH = 2 };
enum { SET_LOOP_SYNCH = 3 };


/**
 * Enables logging for stream purposes
**/
void SetupStreamDebugLogging();

/**
 * Prints any scan information
**/
void PrintScans(int numScans, int numChannels, const char ** channelNames,
	const int * channelAddresses, int deviceScanBacklog, int LJMScanBacklog,
	int iteration, double * aData);

/**
 * Prints a scan of the channels:
 *     "AIN0",  "FIO_STATE",  "SYSTEM_TIMER_20HZ", "STREAM_DATA_CAPTURE_16",
 * appendng SYSTEM_TIMER_20HZ and STREAM_DATA_CAPTURE_16 to create the original
 * 32-bit value of SYSTEM_TIMER_20HZ.
**/
void HardcodedPrintScans(const char ** chanNames, const double * aData,
	int numScansReceived, int numChannelsPerScan, int deviceScanBacklog,
	int LJMScanBacklog);

/**
 * Prints information after stream has finished
**/
void PrintStreamConclusion(unsigned int timeStart, unsigned int timeEnd, int numReads,
	int scansPerRead, int numChannels, int totalSkippedScans);

/**
 * Calulates how many LJM_eStreamRead calls should be done.
 * Para: numSeconds: the desired number of seconds to stream for.
 *       scanRate: the actual scan rate returned from the device.
 *           (LJM_eStreamStart returns this)
 *       scansPerRead: the ScansPerRead parameter of LJM_eStreamStart
**/
int CalculateNumReads(int numSeconds, double scanRate, int scansPerRead);

/**
 * Checks if stream is enabled on the device, then disables it if so
**/
void DisableStreamIfEnabled(int handle);

/**
 * Iterates through aData, totaling LJM_DUMMY_VALUE values. Returns the total.
 * Para: numInChannels, the number of stream in channels.
 *       scansPerRead, the number of scans in one LJM_eStreamRead
 *       aData, the results of one LJM_eStreamRead
**/
int CountAndOutputNumSkippedScans(int numInChannels, int scansPerRead, double * aData);

/**
 * Prints the iteration number, and prints the backlog values if they are greater than their
 * respective thresholds.
**/
void OutputStreamIterationInfo(int iteration, int deviceScanBacklog, int deviceScanBacklogThreshold,
	int LJMScanBacklog, int LJMScanBacklogThreshold);

/**
 * Calculates how much sleep should be done based on how far behind stream is.
**/
double CalculateSleepFactor(int scansPerRead, int LJMScanBacklog);

/**
 * Sleeps for approximately the expected amount of time until the next scan is
 * ready to be read.
**/
void VariableStreamSleep(int scansPerRead, int scanRate, int LJMScanBacklog);

/**
 * Enables externally clocked stream on the device. On the T7, externally clocked
 * stream is read by pulses input to CIO3.
**/
void SetupExternalClockStream(int handle);

/**
 * Enables FIO0 to pulse out for numPulses pulses. This is used in these examples
 * for external stream. This is especially useful for testing external stream -
 * just connect a wire from FIO0 to CIO3 and call this function before starting stream.
 * numPulses should be greater than the expected number of pulses needed for the sake
 * of padding.
**/
void EnableFIO0PulseOut(int handle, int pulseRate, int numPulses);


// Source

void PrintScans(int numScans, int numChannels, const char ** channelNames,
	const int * channelAddresses, int deviceScanBacklog, int LJMScanBacklog,
	int iteration, double * aData)
{
	int scanI, chanI;
	int numSkippedScans = 0;
	const int MAX_NUM = MAX_SCANS_TO_PRINT;
	int limitScans = numScans > MAX_NUM;
	int maxScansPerChannel = limitScans ? MAX_NUM : numScans;
	char * formatString;
	unsigned short temp;
	unsigned char * bytes;

	for (chanI=0; chanI<numChannels; chanI++) {
		if (channelAddresses[chanI] < 1000) {
			formatString = "%10s%14s";
		}
		else {
			formatString = "%10s%24s";
		}
		printf(formatString, channelNames[chanI], "");
	}
	printf("\n");

	for (scanI = 0; scanI < maxScansPerChannel * numChannels; scanI += numChannels) {
		for (chanI=0; chanI<numChannels; chanI++) {
			if (aData[scanI+chanI] == LJM_DUMMY_VALUE) {
				++numSkippedScans;
			}
			if (channelAddresses[chanI] < 1000) {
				printf("aData[%3d]: %+.05f    ", scanI+chanI, aData[scanI + chanI]);
			}
			else {
				temp = (unsigned short) aData[scanI + chanI];
				bytes = (unsigned char *)&temp;
				printf("aData[%3d]: 0x ", scanI + chanI);
				printf("%02x %02x", bytes[0], bytes[1]);
				printf("  (% 7.00f)   ", aData[scanI + chanI]);
			}
		}
		printf("\n");
	}

	if (limitScans) {
		printf("%d scans were omitted from this output.\n", numScans - MAX_NUM);
	}
}

void HardcodedPrintScans(const char ** chanNames, const double * aData,
	int numScansReceived, int numChannelsPerScan, int deviceScanBacklog,
	int LJMScanBacklog)
{
	int dataI, scanI;
	unsigned int timerValue;
	const int NUM_SCANS_TO_PRINT = 1;

	if (numChannelsPerScan < 4 || numChannelsPerScan > 4) {
		printf("unexpected numChannelsPerScan: %d\n", numChannelsPerScan);
		return;
	}

	printf("devBacklog: % 4d - LJMBacklog: % 4d  - %d of %d scans: \n",
		deviceScanBacklog, LJMScanBacklog, NUM_SCANS_TO_PRINT, numScansReceived);
	for (scanI = 0; scanI < NUM_SCANS_TO_PRINT; scanI++) {
		for (dataI = 0; dataI < 2; dataI++) {
			printf(" % 4.03f (%s),", aData[scanI * 4 + dataI], chanNames[dataI]);
		}

		if (strcmp(chanNames[2], "SYSTEM_TIMER_20HZ") != 0
			|| strcmp(chanNames[3], "STREAM_DATA_CAPTURE_16") != 0)
		{
			printf("unexpected register: %s and/or %s\n",
				chanNames[2], chanNames[3]);
			return;
		}

		// Combine CORE_TIMER's lower byte and STREAM_DATA_CAPTURE_16, which
		// contains CORE_TIMER's upper byte
		timerValue = ((unsigned short) aData[scanI * 4 + 3] << 16) +
			(unsigned short) aData[scanI * 4 + 2];
		printf("  0x%8X (%s)", timerValue, chanNames[2]);

		printf("\n");
	}
}

void PrintStreamConclusion(unsigned int timeStart, unsigned int timeEnd, int numReads,
	int scansPerRead, int numChannels, int totalSkippedScans)
{
	double msPerRead = ((double)timeEnd - timeStart) / numReads;

	printf("\nFinished:\n\t%d iterations over approximately %d milliseconds\n",
		numReads, timeEnd - timeStart);
	printf("\t%f ms/read\n", msPerRead);
	printf("\t%f ms/sample\n\n", msPerRead / (scansPerRead * numChannels));

	if (totalSkippedScans) {
		printf("\n****** Total number of skipped scans: %d ******\n\n", totalSkippedScans);
	}
}

void SetupStreamDebugLogging()
{
	// SetConfigString and SetConfigValue are defined in LJM_Utilities.h
	SetConfigString(LJM_DEBUG_LOG_FILE, "default");

	SetConfigValue(LJM_DEBUG_LOG_FILE_MAX_SIZE, 123456789);
	SetConfigValue(LJM_DEBUG_LOG_LEVEL, LJM_STREAM_PACKET);
	SetConfigValue(LJM_DEBUG_LOG_MODE, LJM_DEBUG_LOG_MODE_CONTINUOUS);
}

int CalculateNumReads(int numSeconds, double scanRate, int scansPerRead)
{
	int numReads = numSeconds * scanRate / scansPerRead;
	if (numReads < 1) {
		numReads = 1;
	}

	return numReads;
}

void DisableStreamIfEnabled(int handle)
{
	double firmwareVersion;
	double enabled;
	static const char * name = "STREAM_ENABLE";
	static const int STREAM_NOT_RUNNING = 2620;
	static const char * fwname = "FIRMWARE_VERSION";

	int err = LJM_eReadName(handle, fwname, &firmwareVersion);
	ErrorCheck(err, "LJM_eReadName(Handle=%d, Name=%s, ...)", handle, fwname);

	// T7 FW 1.0024 and lower does not allow read of STREAM_ENABLE
	if (firmwareVersion < 1.0025) {
		printf("Forcing disable of stream for handle: %d\n", handle);
		err = LJM_eStreamStop(handle);
		if (err != LJME_NOERROR && err != STREAM_NOT_RUNNING) {
			ErrorCheck(err, "LJM_eStreamStop(Handle=%d)", handle);
		}
		return;
	}

	err = LJM_eReadName(handle, name, &enabled);
	ErrorCheck(err, "LJM_eReadName(Handle=%d, Name=%s, ...)", handle, name);

	if ((int)enabled) {
		printf("Disabling stream for handle: %d\n", handle);
		err = LJM_eStreamStop(handle);
		PrintErrorIfError(err, "LJM_eStreamStop(Handle=%d)", handle);
	}
}

int CountAndOutputNumSkippedScans(int numInChannels, int scansPerRead, double * aData)
{
	int j;
	int numSkippedScans = 0;
	for (j = 0; j < numInChannels * scansPerRead; j++) {
		if (aData[j] == LJM_DUMMY_VALUE) {
			++numSkippedScans;
		}
	}
	if (numSkippedScans) {
		printf("****** %d data values were placeholders for scans that were skipped ******\n",
			numSkippedScans);
		printf("****** %.01f %% of the scans were skipped ******\n",
			100 * (double)numSkippedScans / scansPerRead / numInChannels);
	}

	return numSkippedScans;
}

void OutputStreamIterationInfo(int iteration, int deviceScanBacklog, int deviceScanBacklogThreshold,
	int LJMScanBacklog, int LJMScanBacklogThreshold)
{
	printf("iteration: %d", iteration);
	if (deviceScanBacklog > deviceScanBacklogThreshold) {
		printf(", deviceScanBacklog: %d", deviceScanBacklog);
	}
	if (LJMScanBacklog > LJMScanBacklogThreshold) {
		printf(", LJMScanBacklog: %d", LJMScanBacklog);
	}
	printf("\n");
}

double CalculateSleepFactor(int scansPerRead, int LJMScanBacklog)
{
	static const double DECREASE_TOTAL = 0.9;
	double portionScansReady = LJMScanBacklog / scansPerRead;
	if (portionScansReady > DECREASE_TOTAL) {
		return 0;
	}
	return (1 - portionScansReady) * DECREASE_TOTAL;
}

void VariableStreamSleep(int scansPerRead, int scanRate, int LJMScanBacklog)
{
	double sleepFactor = CalculateSleepFactor(scansPerRead, LJMScanBacklog);
	int sleepMS = sleepFactor * 1000 * scansPerRead / (double)scanRate;
	if (sleepMS < 1) {
		return;
	}
	MillisecondSleep(sleepMS); // 1000 is to convert to milliseconds
}

void SetupExternalClockStream(int handle)
{
	printf("Setting up externally clocked stream\n");
	WriteNameOrDie(handle, "STREAM_CLOCK_SOURCE", 2);
	WriteNameOrDie(handle, "STREAM_EXTERNAL_CLOCK_DIVISOR", 1);
}

void EnableFIO0PulseOut(int handle, int pulseRate, int numPulses)
{
	// Set FIO0 to do a 50% duty cycle
	// http://labjack.com/support/datasheets/t7/digital-io/extended-features/pulse-out

	int rollValue = 10000000 /* 10 MHz */ / pulseRate;

	printf("Enabling %d pulses on FIO0 at a %d Hz pulse rate\n", numPulses, pulseRate);

	WriteNameOrDie(handle, "DIO0_EF_ENABLE", 0);
	WriteNameOrDie(handle, "DIO_EF_CLOCK0_DIVISOR", 8);
	WriteNameOrDie(handle, "DIO_EF_CLOCK0_ROLL_VALUE", rollValue);
	WriteNameOrDie(handle, "DIO_EF_CLOCK0_ENABLE", 1);
	WriteNameOrDie(handle, "DIO0", 0);
	WriteNameOrDie(handle, "DIO0_EF_TYPE", 2);
	WriteNameOrDie(handle, "DIO0_EF_VALUE_A", 5000);
	WriteNameOrDie(handle, "DIO0_EF_VALUE_B", 0);
	WriteNameOrDie(handle, "DIO0_EF_VALUE_C", numPulses);
	WriteNameOrDie(handle, "DIO0_EF_ENABLE", 1);
}


#endif // #define LJM_STREAM_UTILITIES
