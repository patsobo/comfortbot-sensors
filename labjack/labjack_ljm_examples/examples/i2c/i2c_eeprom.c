/**
 * Name: i2c_eeprom.c
 * Desc: Demonstrates I2C communication using the LJM driver.
 *       The demonstration uses a LJTick-DAC connected to FIO0/FIO1.
 *       This example configures I2C communication on the device, reads user
 *       memory on the LJTick-DAC, writes random bytes to user memory on the
 *       LJTick-DAC, then reads the user memory again.
**/

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "LabJackM.h"

// For LabJackM helper functions
#include "../LJM_Utilities.h"

int main()
{
	int err, handle, i;
	int errorAddress = INITIAL_ERR_ADDRESS;

	// Variables for eNames
	const char * I2C_WRITE_NAMES[1] = {"I2C_WRITE_DATA"};
	const char * I2C_READ_NAMES[1] = {"I2C_READ_DATA"};
	int aWrites[1];
	int aNumValues[1];
	double aValues[32] = {0}; // TX/RX bytes will go here

	// Open first found LabJack
	err = LJM_OpenS("LJM_dtANY", "USB", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Configure the I2C communication.
	err = LJM_eWriteName(handle, "I2C_SDA_DIONUM", 1); // SDA pin number = 1 (FIO1)
	ErrorCheck(err, "LJM_eWriteName (I2C_SDA_DIONUM)");
	err = LJM_eWriteName(handle, "I2C_SCL_DIONUM", 0); // SCA pin number = 0 (FIO0)
	ErrorCheck(err, "LJM_eWriteName (I2C_SCL_DIONUM)");

	// Speed throttle is inversely proportional to clock frequency. 0 = max.
	err = LJM_eWriteName(handle, "I2C_SPEED_THROTTLE", 0);
	ErrorCheck(err, "LJM_eWriteName (I2C_SPEED_THROTTLE)");

	// Options bits:
	//   bit0: Reset the I2C bus.
	//   bit1: Restart w/o stop
	//   bit2: Disable clock stretching.
	err = LJM_eWriteName(handle, "I2C_OPTIONS", 0);
	ErrorCheck(err, "LJM_eWriteName (I2C_OPTIONS)");

	// Slave Address of the I2C chip = 80 (0x50)
	err = LJM_eWriteName(handle, "I2C_SLAVE_ADDRESS", 80);
	ErrorCheck(err, "LJM_eWriteName (I2C_SLAVE_ADDRESS)");

	// Initial read of EEPROM bytes 0-3 in the user memory area. We need a
	// single I2C transmission that writes the chip's memory pointer and then
	// reads the data.
	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_TX", 1); // Set the number of bytes to transmit
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_TX)");

	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_RX", 4); // Set the number of bytes to receive
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_RX)");


	// Set the TX bytes. We are sending 1 byte for the address.
	aWrites[0] = LJM_WRITE; // Indicates we are writing the values.
	aNumValues[0] = 1; // The number of bytes
	aValues[0] = 0; // Byte 0: Memory pointer = 0
	err = LJM_eNames(handle, 1, I2C_WRITE_NAMES, aWrites, aNumValues, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames (I2C_WRITE_DATA)");

	err = LJM_eWriteName(handle, "I2C_GO", 1); // Do the I2C communications.
	ErrorCheck(err, "LJM_eWriteName (I2C_GO)");

	// Read the RX bytes.
	aWrites[0] = LJM_READ; // Indicates we are reading the values.
	aNumValues[0] = 4; // The number of bytes
	for (i = 0; i < 4; i++) {
		aValues[i] = 0;
	}
	err = LJM_eNames(handle, 1, I2C_READ_NAMES, aWrites, aNumValues, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames (I2C_READ_DATA)");

	printf("Read User Memory [0-3] = ");
	for (i = 0; i < 4; i++) {
		printf("%.0f ", aValues[i]);
	}
	printf("\n");


	// Write EEPROM bytes 0-3 in the user memory area, using the page write technique. Note
	// that page writes are limited to 16 bytes max, and must be aligned with the 16-byte
	// page intervals. For instance, if you start writing at address 14, you can only write
	// two bytes because byte 16 is the start of a new page.
	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_TX", 5); // Set the number of bytes to transmit
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_TX)");

	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_RX", 0); // Set the number of bytes to receive
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_RX)");

	// Set the TX bytes.
	aWrites[0] = LJM_WRITE; // Indicates we are writing the values.
	aNumValues[0] = 5; // The number of bytes
	aValues[0] = 0; // Byte 0: Memory pointer = 0
	// Create 4 new random numbers to write (aValues[1-4]).
	srand((unsigned)time(NULL)); // Seeding the random number generator with the current time
	                             // for different values each run.
	for (i = 1; i < 5; i++) {
		aValues[i] = (double)(rand() % 256); // "% 256" gives the range 0 to 255
	}
	err = LJM_eNames(handle, 1, I2C_WRITE_NAMES, aWrites, aNumValues, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames (I2C_WRITE_DATA)");

	err = LJM_eWriteName(handle, "I2C_GO", 1); // Do the I2C communications.
	ErrorCheck(err, "LJM_eWriteName (I2C_GO)");

	printf("Write User Memory [0-3] = ");
	for (i = 1; i < 5; i++) {
		printf("%.0f ", aValues[i]);
	}
	printf("\n");


	// Final read of EEPROM bytes 0-3 in the user memory area.
	// We need a single I2C transmission that writes the address and then reads
	// the data.
	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_TX", 1); // Set the number of bytes to transmit
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_TX)");

	err = LJM_eWriteName(handle, "I2C_NUM_BYTES_RX", 4); // Set the number of bytes to receive
	ErrorCheck(err, "LJM_eWriteName (I2C_NUM_BYTES_RX)");

	// Set the TX bytes. We are sending 1 byte for the address.
	aWrites[0] = LJM_WRITE; // Indicates we are writing the values.
	aNumValues[0] = 1; // The number of bytes
	aValues[0] = 0; // Byte 0: Memory pointer = 0
	err = LJM_eNames(handle, 1, I2C_WRITE_NAMES, aWrites, aNumValues, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames (I2C_WRITE_DATA)");

	err = LJM_eWriteName(handle, "I2C_GO", 1); // Do the I2C communications.
	ErrorCheck(err, "LJM_eWriteName (I2C_GO)");

	// Read the RX bytes.
	aWrites[0] = LJM_READ; // Indicates we are reading the values.
	aNumValues[0] = 4; // The number of bytes
	for (i = 0; i < 4; i++) {
		aValues[i] = 0;
	}
	err = LJM_eNames(handle, 1, I2C_READ_NAMES, aWrites, aNumValues, aValues, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eNames (I2C_READ_DATA)");

	printf("Read User Memory [0-3] = ");
	for (i = 0; i < 4; i++) {
		printf("%.0f ", aValues[i]);
	}
	printf("\n");

	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
