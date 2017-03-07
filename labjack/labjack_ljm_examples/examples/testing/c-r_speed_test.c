/**
 * Name: c-r_speed_test.c
 * Desc: Performs LabJack operations in a loop and reports the timing
 *       statistics for the operations.
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include "LabJackM.h"

// For LabJackM helper functions
#include "../LJM_Utilities.h"

#define FALSE 0
#define TRUE 1

/**
 * Desc: Configures AIN settings on the device. On error, prints error, closes all
 *       devices, and exits the program.
 * Para: handle, the device to configure
 *       numAIN, the number of AINs to configure. For each numAIN, configures starting
 *               with AIN 0 and ends with AIN (numAIN minus 1). Example: numAIN == 3,
 *               AIN0, AIN1, and AIN2 are configured
 *       rangeAIN, the range to set each AIN to
 *       resolutionAIN, the resolution to set each AIN to
 *       settlingIndexAIN, the settling index to set each AIN to
 * Note: numAIN must be be greater than 0
**/
void ConfigureAIN(int handle, int numAIN, double rangeAIN, double resolutionAIN,
	double settlingIndexAIN);

/**
 * Desc: Sets up parameters to be passed to LJM_eNames. From the input parameters,
 *       counts the number of frames, allocates memory for the arrays, and initializes
 *       the arrays. On error, prints error, closes all devices, and exits the program.
 * Para: numAIN, the number of AINs to read. For each numAIN, adds 1 read frame, starting
 *               with AIN 0 and ending with AIN (numAIN minus 1). Example: numAIN == 3,
 *               AIN0, AIN1, and AIN2 are read
 *       writeDACs, FALSE/0 for false, non-zero for true. If true, adds 2 frames that
 *                  write 0.0 to both DAC0 and DAC1
 *       readDigital, FALSE/0 for false, non-zero for true. If true, adds 1 frame that
 *                    reads DIO_STATE
 *       writeDigital, FALSE/0 for false, non-zero for true. If true, adds 1 frame that
 *                     writes 0.0 to DIO_STATE
 *       numFrames, output parameter. Gives the total number of frames created
 *       aNames, output array. Gives the names of each frame
 *       aWrites, output array. Gives the direction of each frame (read/write)
 *       aNumValues, output array. Always 1 for each frame
 *       aValues, output array. Gives the value of each frame
**/
void SetUpSpeedTest(int numAIN, int writeDACs, int readDigital, int writeDigital,
	int * numFrames, char *** aNames, int ** aWrites, int ** aNumValues, double ** aValues);

/**
 * Desc: Deallocates memory for the arrays
**/
void CleanUpSpeedTest(int numFrames, char ** aNames, int * aWrites, int * aNumValues,
	double * aValues);

/**
 * Desc: Calls LJM_eNames in a loop, recording how fast the loop takes to complete, and
 *       prints the results. On error, prints error, closes all devices, and exits the program.
 * Para: numIterations, the number of times to call LJM_eNames
**/
void SpeedTest(int numIterations, int handle, int numFrames, const char ** aNames,
	int * aWrites, int * aNumValues, double * aValues);

int main()
{
	// Number of iterations to perform in the loop
	const int NUM_ITERATIONS = 1000;

	// Analog input settings for this test
	const int NUM_AIN = 1;
	double rangeAIN = 10.0;
	double resolutionAIN = 1;
	double settlingIndexAIN = 0;

	// Analog output settings
	int writeDACs = FALSE;

	// Digital input/output settings
	int readDigital = FALSE;
	int writeDigital = FALSE;


	int numFrames;
	char ** aNames;
	int * aWrites;
	int * aNumValues;
	double * aValues;

	int err;
	int handle;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);
	printf("\n");

	if (NUM_AIN > 0) {
		ConfigureAIN(handle, NUM_AIN, rangeAIN, resolutionAIN, settlingIndexAIN);
	}

	SetUpSpeedTest(NUM_AIN, writeDACs, readDigital, writeDigital,
		&numFrames, &aNames, &aWrites, &aNumValues, &aValues);

	SpeedTest(NUM_ITERATIONS, handle, numFrames, (const char **)aNames, aWrites,
		aNumValues, aValues);

	CleanUpSpeedTest(numFrames, aNames, aWrites, aNumValues, aValues);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void ConfigureAIN(int handle, int numAIN, double rangeAIN, double resolutionAIN,
	double settlingIndexAIN)
{
	int i = 0;
	enum { NUM_FRAMES = 3 };
	char range[LJM_MAX_NAME_SIZE];
	char resolutionIndex[LJM_MAX_NAME_SIZE];
	char settling[LJM_MAX_NAME_SIZE];
	char * aNames[NUM_FRAMES] = {range, resolutionIndex, settling};
	double aValues[NUM_FRAMES];
	int errorAddress = INITIAL_ERR_ADDRESS;
	int err = 0;

	if (numAIN < 1) {
		printf("ConfigureAIN: numAIN must be greater than 0\n");
		LJM_CloseAll();
		exit(1);
	}

	for (i=0; i<numAIN; i++) {
		sprintf(range, "AIN%d_RANGE", i);
		aValues[0] = rangeAIN;

		sprintf(resolutionIndex, "AIN%d_RESOLUTION_INDEX", i);
		aValues[1] = resolutionAIN;

		sprintf(settling, "AIN%d_SETTLING_US", i);
		aValues[2] = settlingIndexAIN;

		err = LJM_eWriteNames(handle, NUM_FRAMES, (const char **)aNames, aValues, &errorAddress);
		ErrorCheckWithAddress(err, errorAddress, "ConfigureAIN: LJM_eWriteNames");
	}
}

void SetUpSpeedTest(int numAIN, int writeDACs, int readDigital, int writeDigital,
	int * numFramesPtr, char *** aNames, int ** aWrites, int ** aNumValues, double ** aValues)
{
	int i = 0;
	int frame = 0;

	// Count how many frames we need
	int numFrames = numAIN;
	if (writeDACs)
		numFrames += 2;
	if (readDigital)
		++numFrames;
	if (writeDigital)
		++numFrames;

	// Allocate for how many frames we need
	*aNames = (char **) malloc (numFrames * sizeof(char *));
	*aWrites = (int *) malloc (numFrames * sizeof(int));
	*aNumValues = (int *) malloc (numFrames * sizeof(int));
	*aValues = (double *) malloc (numFrames * sizeof(double));

	frame = 0;
	for (i=0; i<numAIN; i++) {
		(*aNames)[frame] = (char *) malloc (LJM_MAX_NAME_SIZE * sizeof(char));
		sprintf((*aNames)[frame], "AIN%d", i);
		(*aWrites)[frame] = LJM_READ;
		(*aNumValues)[frame] = 1;
		(*aValues)[frame] = 0;
		++frame;
	}

	if (readDigital) {
		(*aNames)[frame] = (char *) malloc (LJM_MAX_NAME_SIZE * sizeof(char));
		sprintf((*aNames)[frame], "DIO_STATE");
		(*aWrites)[frame] = LJM_READ;
		(*aNumValues)[frame] = 1;
		(*aValues)[frame] = 0;
		++frame;
	}

	if (writeDigital) {
		(*aNames)[frame] = (char *) malloc (LJM_MAX_NAME_SIZE * sizeof(char));
		sprintf((*aNames)[frame], "DIO_STATE");
		(*aWrites)[frame] = LJM_WRITE;
		(*aNumValues)[frame] = 1;
		(*aValues)[frame] = 0; // output-low
		++frame;
	}

	if (writeDACs) {
		for (i=0; i<2; i++) {
			(*aNames)[frame] = (char *) malloc (LJM_MAX_NAME_SIZE * sizeof(char));
			sprintf((*aNames)[frame], "DAC%d", i);
			(*aWrites)[frame] = LJM_WRITE;
			(*aNumValues)[frame] = 1;
			(*aValues)[frame] = 0.0; // 0.0 V
			++frame;
		}
	}

	*numFramesPtr = numFrames;
}

void CleanUpSpeedTest(int numFrames, char ** aNames, int * aWrites, int * aNumValues,
	double * aValues)
{
	int i;
	for (i=0; i<numFrames; i++) {
		free (aNames[i]);
	}
	free (aNames);
	free (aWrites);
	free (aNumValues);
	free (aValues);
}

void SpeedTest(int numIterations, int handle, int numFrames, const char ** aNames,
	int * aWrites, int * aNumValues, double * aValues)
{
	unsigned int timeStart, timeEnd, totalMS;
	int errorAddress = INITIAL_ERR_ADDRESS;
	int err;
	int i;
	const char * READ = "read";
	const char * WRITE = "write";
	const char * readOrWrite;

	printf("Speed test frames:\n");
	for (i=0; i<numFrames; i++) {
		if (aWrites[i] == LJM_READ) {
			readOrWrite = READ;
		}
		else {
			readOrWrite = WRITE;
		}

		printf("    %s %s\n", readOrWrite, aNames[i]);
	}

	printf("\nBeginning %d iterations...\n", numIterations);

	timeStart = GetCurrentTimeMS();
	for (i=0; i<numIterations; i++) {
		// Note: "Address" functions are faster than "Name" functions because
		// they don't have to match a name to an address/type
		err = LJM_eNames(handle, numFrames, aNames, aWrites, aNumValues, aValues,
			&errorAddress);
		ErrorCheckWithAddress(err, errorAddress, "LJM_eNames");
	}
	timeEnd = GetCurrentTimeMS();

	totalMS = timeEnd - timeStart;

	printf("%d iterations performed\n", numIterations);
	printf("    Approximate time taken: %d ms\n", totalMS);
	printf("    Approximate average time per iteration: %f ms\n",
		((double)totalMS)/numIterations);

	printf("\nLast results:\n");
	for (i=0; i<numFrames; i++) {
		if (aWrites[i] == LJM_READ) {
			readOrWrite = READ;
		}
		else {
			readOrWrite = WRITE;
		}

		printf("    %s %s value : %f\n", aNames[i], readOrWrite, aValues[i]);
	}
}
