/**
 * Name: read_cal.c
 * Desc: Reads and displays device calibration information
**/

// For printf
#include <stdio.h>

// For the LabJackM library
#include "LabJackM.h"

// For LabJackM helper functions
#include "../LJM_Utilities.h"

/**
 * Desc: Reads and displays calibration information for a LabJack T7. If there is
 *       an error, prints the error, closes all devices, and exits the program.
**/
void ReadT7Cal(int handle);

int main()
{
	int err, handle, deviceType;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	err = LJM_GetHandleInfo(handle, &deviceType, NULL, NULL, NULL, NULL, NULL);
	ErrorCheck(err, "LJM_GetHandleInfo");

	if (deviceType != LJM_dtT7) {
		printf("Device type %d is not supported in this program.\n", deviceType);
		LJM_CloseAll();
		WaitForUserIfWindows();
		return 1;
	}

	printf("\n");

	ReadT7Cal(handle);


	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

typedef struct {
	float PSlope;
	float NSlope;
	float Center;
	float Offset;
} Cal_Set;
typedef struct{
	Cal_Set HS[4];
	Cal_Set HR[4];

	struct{
		float Slope;
		float Offset;
	}DAC[2];

	float Temp_Slope;
	float Temp_Offset;

	float ISource_10u;
	float ISource_200u;

	float I_Bias;
}DeviceCalibrationT7;

void ReadT7Cal(int handle)
{
	int err, valueI;
	int errorAddress = INITIAL_ERR_ADDRESS;

	const double EFAdd_CalValues = 0x3C4000;

	const int FLASH_PTR_ADDRESS = 61810;
	const int FLASH_PTR_TYPE = LJM_UINT32;

	// 3 frames of 13 values, one frame of 2 values
	enum { NUM_FRAMES = 4 };
	const int FLASH_READ_ADDRESSES[1] = {61812};
	const int FLASH_READ_TYPES[1] = {LJM_FLOAT32};
	const int FLASH_READ_DIRECTIONS[1] = {LJM_READ};
	const int FLASH_READ_NUM_VALUES[NUM_FRAMES] = {13, 13, 13, 2};
	enum { NUM_VALUES = 41 };
	double calValues[NUM_VALUES] = {0.0};

	DeviceCalibrationT7 cal;

	int i;
	for (i=0; i<NUM_FRAMES; i++) {
		// Set the pointer. This indicates which part of the memory we want to read
		err = LJM_eWriteAddress(handle, FLASH_PTR_ADDRESS, FLASH_PTR_TYPE, EFAdd_CalValues + i * 13 * 4);
		ErrorCheck(err, "LJM_eWriteAddress(..., %d, %d, %f)", FLASH_PTR_ADDRESS, FLASH_PTR_TYPE,
			EFAdd_CalValues);

		err = LJM_eAddresses(handle, 1, FLASH_READ_ADDRESSES, FLASH_READ_TYPES,
			FLASH_READ_DIRECTIONS, &(FLASH_READ_NUM_VALUES[i]), calValues + i * 13,
			&errorAddress);
		ErrorCheckWithAddress(err, errorAddress, "LJM_eAddresses");
	}

	// Copy to our cal constantss structure
	for (valueI=0; valueI<NUM_VALUES; valueI++) {
		((float *)&cal)[valueI] = calValues[valueI];
	}

	printf("Cal values:\n");
	for (i=0; i<4; i++) {
		printf("HS[%d]:\n", i);
		printf("    PSlope: %+.10f\n", cal.HS[i].PSlope);
		printf("    NSlope: %+.10f\n", cal.HS[i].NSlope);
		printf("    Center: %+f\n", cal.HS[i].Center);
		printf("    Offset: %+f\n", cal.HS[i].Offset);
	}

	for (i=0; i<4; i++) {
		printf("HR[%d]:\n", i);
		printf("    PSlope: %+.10f\n", cal.HR[i].PSlope);
		printf("    NSlope: %+.10f\n", cal.HR[i].NSlope);
		printf("    Center: %+f\n", cal.HR[i].Center);
		printf("    Offset: %+f\n", cal.HR[i].Offset);
	}

	for (i=0; i<2; i++) {
		printf("DAC[%d]:\n", i);
		printf("    Slope:  %+f\n", cal.DAC[i].Slope);
		printf("    Offset: %+f\n", cal.DAC[i].Offset);
	}

	printf("Temp:\n");
	printf("    Temp_Slope:  %+f\n", cal.Temp_Slope);
	printf("    Temp_Offset: %+f\n", cal.Temp_Offset);

	printf("ISource:\n");
	printf("    ISource_10u:  %+.10f\n", cal.ISource_10u);
	printf("    ISource_200u: %+.10f\n", cal.ISource_200u);

	printf("I_Bias: %+.10f\n", cal.I_Bias);
}
