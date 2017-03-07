/**
 * Name: names_to_addresses.c
 * Desc: Shows LJM_NamesToAddresses in action
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memset and strncpy

#include "../LJM_Utilities.h"

#include "LabJackM.h"

void ErrorDisplay(int err)
{
	char errName[LJM_MAX_NAME_SIZE];
	LJM_ErrorToString(err, errName);
	if(err != LJME_NOERROR)
	{
		printf("Error: %d (%s)\n", err, errName);
	}
}

void PrintNamesToAddressOutput(int NumFrames, const char ** Names, int * aAddresses,
	int * aTypes, int err)
{
	int i;
	for (i=0; i<NumFrames; i++)
	{
		printf("%s - address: %d, type: %d\n", Names[i], aAddresses[i], aTypes[i]);
	}

	ErrorDisplay(err);
}

void PrintRegisterConstantsExample();

int main()
{
	int err;

	// Should work
	#define THE_SIZE 2
	const char *(Names)[] = {"DAC0", "SERIAL_NUMBER"};
	int aAddresses[THE_SIZE];
	int aTypes[THE_SIZE];
	int NumFrames = THE_SIZE;

	// Should fail
	const char *(NamesBad)[] = {"NOT_A_VALID_REGISTER_NAME", "SERIAL_NUMBER"};

	// Dynamically allocated
	char ** namesDyn;

	printf("Here is an example that should work:\n");
	err = LJM_NamesToAddresses(NumFrames, Names, aAddresses, aTypes);
	PrintNamesToAddressOutput(NumFrames, Names, aAddresses, aTypes, err);
	printf("\n");


	printf("Here the first name fails, so the first address becomes equal to LJM_INVALID_NAME_ADDRESS:\n");
	err = LJM_NamesToAddresses(NumFrames, NamesBad, aAddresses, aTypes);
	PrintNamesToAddressOutput(NumFrames, NamesBad, aAddresses, aTypes, err);
	printf("\n");


	printf("Here is the dynamically allocated strings example:\n");
	// Allocate the array to hold the strings
	namesDyn = (char**) malloc(NumFrames * sizeof(char*));

	// Allocate the strings
	// Make sure to malloc enough for the extra null-termination character
	namesDyn[0] = (char*) malloc(20 * sizeof(char));
	namesDyn[1] = (char*) malloc(20 * sizeof(char));

	// Not necessary, but keeps things safe
	memset(namesDyn[0], '\0', 20);
	memset(namesDyn[1], '\0', 20);

	// Copy the names to the string array
	strncpy(namesDyn[0], "DAC0", 20);
	strncpy(namesDyn[1], "SERIAL_NUMBER", 20);

	err = LJM_NamesToAddresses(NumFrames, Names, aAddresses, aTypes);
	PrintNamesToAddressOutput(NumFrames, Names, aAddresses, aTypes, err);
	printf("\n");

	// Free the memory from malloc
	free(namesDyn[0]);
	free(namesDyn[1]);
	free(namesDyn);

	PrintRegisterConstantsExample();

	WaitForUserIfWindows();

	return LJME_NOERROR;
}

void PrintRegisterConstantsExample()
{
	char * Scope = "WIFI_STATUS";
	char ConstantName[LJM_MAX_NAME_SIZE];
	double ConstantValue;
	int err;

	printf("Here is an example of LJM_LookupConstantValue and LJM_LookupConstantName:\n");

	strncpy(ConstantName, "ASSOCIATED", LJM_MAX_NAME_SIZE);
	err = LJM_LookupConstantValue(Scope, ConstantName, &ConstantValue);
	ErrorCheck(err, "LJM_LookupConstantValue(%s, %s, ...)", Scope, ConstantName);
	printf("Scope: %s, ConstantName: %s  =>  ConstantValue: %.00f\n", Scope, ConstantName, ConstantValue);

	ConstantValue = 2901;
	err = LJM_LookupConstantName(Scope, ConstantValue, ConstantName);
	ErrorCheck(err, "LJM_LookupConstantName(%s, %f, ...)", Scope, ConstantValue);
	printf("Scope: %s, ConstantValue: %.00f  =>  ConstantName: %s\n", Scope, ConstantValue, ConstantName);
	printf("\n");
}
