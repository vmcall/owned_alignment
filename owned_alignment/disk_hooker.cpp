#include "disk_hooker.hpp"
#include <cstdint>
#include "physical_memory.hpp"
#include "driver.hpp"
#include "ioctl_helper.hpp"
#include "nt_ioctl.hpp"

uint8_t disk_hooker::xor_key = 0;
PDRIVER_DISPATCH disk_hooker::g_original_disk_dispatch = nullptr;
PDRIVER_DISPATCH disk_hooker::g_original_scsi_dispatch = nullptr;

bool disk_hooker::hook_disk_driver()
{
	auto disk_driver = driver::find_from_name(L"\\Driver\\Disk", false);	
	if (!disk_driver.valid())
	{
		DbgPrint("Failed to find driver.\n");
		return false;
	}

	disk_driver.hook_ioctl<true>(disk_hooker::g_original_disk_dispatch, [](DEVICE_OBJECT* device_object, IRP* irp){

		auto current_stack_location = irp->Tail.Overlay.CurrentStackLocation;
		const auto control_code = current_stack_location->Parameters.DeviceIoControl.IoControlCode;
		switch (control_code)
		{
		case IOCTL_STORAGE_QUERY_PROPERTY:
		{
			DbgPrint("[dispatch_hook] IOCTL_STORAGE_QUERY_PROPERTY\n");

			auto property_query = reinterpret_cast<STORAGE_PROPERTY_QUERY*>(irp->AssociatedIrp.SystemBuffer);

			if (property_query && property_query->PropertyId == StorageDeviceProperty)
				ioctl_helper::set_completion_callback(irp, current_stack_location, disk_hooker::storage_query_callback);

			break;
		}
		case DFP_READ_VERSION:
		{
			DbgPrint("[dispatch_hook] DFP_READ_VERSION\n");
			break;
		}
		case SMART_RCV_DRIVE_DATA:
		{
			DbgPrint("[dispatch_hook] SMART_RCV_DRIVE_DATA, handle in SCSI hook!\n");

			break;
		}
		default:
		{
			if (control_code)
				DbgPrint("[dispatch_hook] Control code: %lx\n", control_code);

			break;
		}

		}

		return g_original_disk_dispatch(device_object, irp);
	});

	return true;
}


bool disk_hooker::hook_scsi_driver()
{
	auto scsi_device = driver::find_device(L"\\Device\\ScsiPort0");

	if (!scsi_device)
	{
		DbgPrint("Failed to find device.\n");
		return false;
	}

	auto scsi_driver = driver(scsi_device->DriverObject);

	if (!scsi_driver.valid())
	{
		DbgPrint("Failed to find driver.\n");
		return false;
	}

	scsi_driver.hook_ioctl<true>(disk_hooker::g_original_scsi_dispatch, [](DEVICE_OBJECT* device_object, IRP* irp) {

		auto current_stack_location = irp->Tail.Overlay.CurrentStackLocation;
		const auto control_code = current_stack_location->Parameters.DeviceIoControl.IoControlCode;
		switch (control_code)
		{
		case IOCTL_SCSI_MINIPORT:
		{
			DbgPrint("[dispatch_hook] IOCTL_SCSI_MINIPORT\n");

			auto miniport_query = reinterpret_cast<SRB_IO_CONTROL*>(irp->AssociatedIrp.SystemBuffer);

			if (miniport_query->ControlCode == IOCTL_SCSI_MINIPORT_IDENTIFY)
			{
				DbgPrint("[dispatch_hook] IOCTL_SCSI_MINIPORT_IDENTIFY\n");
				ioctl_helper::set_completion_callback(irp, current_stack_location, disk_hooker::scsi_callback);
			}

			break;
		}
		default:
		{
			if (control_code)
				DbgPrint("[dispatch_hook] Control code: %lx\n", control_code);

			break;
		}

		}

		return g_original_scsi_dispatch(device_object, irp);
	});

	return true;
}

void disk_hooker::setup_xor()
{
	// SET UP XOR KEY
	disk_hooker::xor_key = static_cast<uint8_t>(__rdtsc());
	DbgPrint("Xor key: %x\n", disk_hooker::xor_key);
}


NTSTATUS disk_hooker::storage_query_callback(DEVICE_OBJECT* device_object, IRP* irp, void* context)
{
	if (context == nullptr)
	{
		DbgPrint("storage_query_callback context == nullptr\n");
		return STATUS_SUCCESS;
	}

	// GET CALLBACK DATA
	const auto data = ioctl_helper::fetch_callback_data<STORAGE_DEVICE_DESCRIPTOR>(context);

	if (data.buffer_length < FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR, RawDeviceProperties))
	{
		DbgPrint("Requested size\n");
	}
	else if (data.buffer->SerialNumberOffset < FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR, RawDeviceProperties))
	{
		DbgPrint("Device doesn't have a serial\n");
	} 
	else if (data.buffer_length < FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR, RawDeviceProperties) + data.buffer->RawPropertiesLength || data.buffer->SerialNumberOffset >= data.buffer_length)
	{
		DbgPrint("Unexpected buffer error\n");
	}
	else
	{
		auto serial = reinterpret_cast<unsigned char*>(data.buffer) + data.buffer->SerialNumberOffset;
		DbgPrint("Old serial: %s\n", serial);

		disk_hooker::spoof_serial(serial);

		DbgPrint("New serial: %s\n", serial);
	}

	// CALL NEXT CALLBACK
	if (irp->StackCount > 1ul && data.old_routine)
		return data.old_routine(device_object, irp, data.old_context);


	return STATUS_SUCCESS;
}


NTSTATUS disk_hooker::scsi_callback(DEVICE_OBJECT* device_object, IRP* irp, void* context)
{
	if (context == nullptr)
	{
		DbgPrint("scsi_callback context == nullptr\n");
		return STATUS_SUCCESS;
	}

	// GET CALLBACK DATA
	const auto data = ioctl_helper::fetch_callback_data<SENDCMDOUTPARAMS>(context);

	if (data.buffer_length < FIELD_OFFSET(SENDCMDOUTPARAMS, bBuffer)
		|| FIELD_OFFSET(SENDCMDOUTPARAMS, bBuffer) + data.buffer->cBufferSize > data.buffer_length
		|| data.buffer->cBufferSize < sizeof(IDINFO)
		)
	{
		DbgPrint("Unexpected buffer error\n");
	}
	else
	{
		const auto params = reinterpret_cast<SENDCMDOUTPARAMS*>(data.buffer->bBuffer + sizeof(SRB_IO_CONTROL));
		const auto info = reinterpret_cast<IDINFO*>(params->bBuffer);

		auto serial = reinterpret_cast<unsigned char*>(info->sSerialNumber);

		DbgPrint("Old serial: %s\n", serial);

		disk_hooker::spoof_serial(serial);

		DbgPrint("New serial: %s\n", serial);
	}

	// CALL NEXT CALLBACK
	if (irp->StackCount > 1ul && data.old_routine)
		return data.old_routine(device_object, irp, data.old_context);

	return STATUS_SUCCESS;
}
