/**
 * Name: o_stream_update.c
 * Desc: This program can be run while o_stream_run is running, and will modify
 *       the running Stream Out.
 * Note: This program is meaningless to run if the device is not running a
 *       Stream Out.
**/

#include "../LJM_Utilities.h"

void UpdateStreamOut(int handle)
{
	WriteNameOrDie(handle, "STREAM_OUT0_LOOP_SIZE", 1);
	WriteNameOrDie(handle, "STREAM_OUT0_BUFFER_F32", 1.23);
	WriteNameOrDie(handle, "STREAM_OUT0_SET_LOOP", 1);
}

int main()
{
	int handle;

	handle = OpenOrDie(LJM_dtT7, LJM_ctUSB, "LJM_idANY");
	UpdateStreamOut(handle);

	CloseOrDie(handle);

	return LJME_NOERROR;
}
