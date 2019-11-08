#pragma once
#include "nt.hpp"
#include "nt_ioctl.hpp"

namespace disk_hooker
{
	// HOOK DISK DRIVER IOCTL
	bool hook_disk_driver();

	// HOOK SCSI IOCTL
	bool hook_scsi_driver();

	// XOR KEY GENERATED FORM CPU CLOCK TIME, USED TO SPOOF SERIAL
	void setup_xor();
	extern uint8_t xor_key;

	// IOCTL HOOK
	extern PDRIVER_DISPATCH g_original_disk_dispatch;
	extern PDRIVER_DISPATCH g_original_scsi_dispatch;

	// IRP CALLBACKS, THESE ARE CALLED BEFORE IOCTL RETURNS VALUE TO CALLER
	NTSTATUS storage_query_callback(DEVICE_OBJECT* device_object, IRP* irp, void* context);
	//NTSTATUS smart_query_callback(DEVICE_OBJECT* device_object, IRP* irp, void* context);
	NTSTATUS scsi_callback(DEVICE_OBJECT* device_object, IRP* irp, void* context);

	// SPOOFING ROUTINE
	__forceinline void spoof_serial(unsigned char* serial)
	{
		// SPOOF
		for (uint8_t i = 0; serial[i] != 0x00; i++)
		{
			serial[i] = (disk_hooker::xor_key + serial[i]) * i;	// GENERATE RANDOM VALUE FROM INDEX, VALUE AND XOR KEY
			serial[i] %= 'Z' - 'A';								// LIMIT TO A-Z RANGE
			serial[i] += 'A';									// ADD BASE TO DELTA
		}
	}
}