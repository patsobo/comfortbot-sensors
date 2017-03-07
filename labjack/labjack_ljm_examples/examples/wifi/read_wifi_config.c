/**
 * Name: read_wifi_config.c
 * Desc: Demonstrates how to read the Wifi configuration from a LabJack.
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include "LabJackM.h"

// For LabJackM helper functions
#include "../LJM_Utilities.h"

int main()
{
	int err;
	int handle;
	int i;
	int errorAddress = INITIAL_ERR_ADDRESS;

	// Set up read IPs operation
	enum { NUM_IP_FRAMES = 6 };
	const char * aNamesIP[NUM_IP_FRAMES] = {
		"WIFI_IP", "WIFI_SUBNET", "WIFI_GATEWAY",
		"WIFI_IP_DEFAULT", "WIFI_SUBNET_DEFAULT", "WIFI_GATEWAY_DEFAULT"};
	double aValuesIP[NUM_IP_FRAMES] = {0};
	char IPv4String[LJM_IPv4_STRING_SIZE];

	// Set up read DHCP_ENABLE operation
	enum { NUM_ENABLE_FRAMES = 2};
	const char * aNamesEnable[NUM_ENABLE_FRAMES] = {"WIFI_DHCP_ENABLE",
		"WIFI_DHCP_ENABLE_DEFAULT"};
	double aValuesEnable[NUM_ENABLE_FRAMES] = {0};

	const char * STATUS_REGISTER = "WIFI_STATUS";
	double status;

	// Open first found LabJack
	err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &handle);
	// err = LJM_OpenS("LJM_dtANY", "LJM_ctANY", "LJM_idANY", &handle);
	ErrorCheck(err, "LJM_Open");

	PrintDeviceInfoFromHandle(handle);

	// Read config values
	err = LJM_eReadNames(handle, NUM_IP_FRAMES, aNamesIP, aValuesIP, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadNames - IP registers");

	err = LJM_eReadNames(handle, NUM_ENABLE_FRAMES, aNamesEnable, aValuesEnable, &errorAddress);
	ErrorCheckWithAddress(err, errorAddress, "LJM_eReadNames - Enable registers");

	err = LJM_eReadName(handle, STATUS_REGISTER, &status);
	ErrorCheck(err, "LJM_eReadName - %s", STATUS_REGISTER);

	printf("\nWifi configurations:\n");
	for (i=0; i<NUM_IP_FRAMES; i++) {
		err = LJM_NumberToIP((unsigned int)aValuesIP[i], IPv4String);
		ErrorCheck(err, "LJM_NumberToIP");

		printf("    %s : %s\n", aNamesIP[i], IPv4String);
	}

	for (i=0; i<NUM_ENABLE_FRAMES; i++) {
		printf("    %s : %.0f\n", aNamesEnable[i], aValuesEnable[i]);
	}

	printf("    %s : %.0f\n", STATUS_REGISTER, status);

	// Close
	err = LJM_Close(handle);
	ErrorCheck(err, "LJM_Close");

	WaitForUserIfWindows();

	return LJME_NOERROR;
}
