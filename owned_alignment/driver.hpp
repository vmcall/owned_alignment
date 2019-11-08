#pragma once
#include "nt.hpp"
#include "shellcode_generator.hpp"
#include "portable_executable.hpp"

class driver
{
public:
	explicit driver(DRIVER_OBJECT* new_driver_obj) 
	{
		this->driver_obj = referenced_object<DRIVER_OBJECT>();
		this->driver_obj.reset(new_driver_obj);
	}

	template<size_t N>
	[[nodiscard]]
	__forceinline static driver find_from_name(const wchar_t(&name)[N], bool is_device)
	{
		UNICODE_STRING uname;
		uname.Buffer = const_cast<wchar_t*>(name);
		uname.Length = uname.MaximumLength = (N - 1) * sizeof(wchar_t);

		if (is_device)
		{
			DEVICE_OBJECT* device_object;
			ObReferenceObjectByName(&uname,
				OBJ_CASE_INSENSITIVE,
				nullptr,
				0,
				IoDeviceObjectType,
				KernelMode,
				nullptr,
				reinterpret_cast<void**>(&device_object));

			DbgPrint("Device: %p\n", device_object);

			return device_object ? driver(device_object->DriverObject) : driver(nullptr);
		}
		else
		{
			DRIVER_OBJECT* driver_object = nullptr;
			ObReferenceObjectByName(&uname,
				OBJ_CASE_INSENSITIVE,
				nullptr,
				0,
				IoDriverObjectType,
				KernelMode,
				nullptr,
				reinterpret_cast<void**>(&driver_object));

			return driver(driver_object);
		}

	}

	template <size_t N>
	[[nodiscard]]
	__forceinline static DEVICE_OBJECT* find_device(const wchar_t(&name)[N])
	{
		();

		UNICODE_STRING uname;
		uname.Buffer = const_cast<wchar_t*>(name);
		uname.Length = uname.MaximumLength = (N - 1) * sizeof(wchar_t);

		DEVICE_OBJECT* device_object = nullptr;
		FILE_OBJECT* file = nullptr;

		IoGetDeviceObjectPointer(&uname, 0, &file, &device_object);

		return device_object;
	}

	// WRAPPERS
	portable_executable get_pe();

	// DRIVER OBJECT DATA
	uint8_t* base_address();
	uint64_t size();
	DRIVER_OBJECT* get_object();
	PDRIVER_DISPATCH* major_function(int index);

	// SANITY
	bool valid();

	// MODULES

	template <class Fn>
	__forceinline void enumerate_function_calls(Fn fn)
	{
		auto text_section = this->get_pe().get_section(".text");

		for (auto data_pointer = text_section.start;
			data_pointer < text_section.start + text_section.size - 5/*SIZE OF CALL*/;
			data_pointer++)
		{
			// CALL INSTRUCTION
			if (*data_pointer == 0xE8)
			{
				// READ CALL TARGET
				std::uint8_t* operand = data_pointer + *reinterpret_cast<int32_t*>(data_pointer + 1) + 5;

				if (operand < this->base_address() ||
					operand > this->base_address() + this->size())
				{
					if (fn(operand, data_pointer))
						break;
				}
			}
		}
	}

	template <bool trampoline>
	__forceinline bool hook_ioctl(PDRIVER_DISPATCH& original_handler, PDRIVER_DISPATCH fn)
	{
		// FIND TRAMPOLINE INSIDE OF DRIVER TEXT SECTION
		if constexpr(trampoline)
		{
			uint8_t* trampoline_location = nullptr;
			uint8_t* detour_location = nullptr;

			this->enumerate_function_calls([&](uint8_t* operand, uint8_t* data_pointer) {

				DbgPrint("Found call to %llx at %llx\n", operand, data_pointer);

				const auto table_info = physical_memory::query_hardware_table_information(operand);

				if (table_info.pde == nullptr)
				{
					DbgPrint("Call destination has invalid PXE/PPE :(\n");
					return false;
				}

				DbgPrint("Call destination has valid PXE/PPE\n");

				if (physical_memory::is_valid_address(operand))
				{
					DbgPrint("Call destination is already in use :(\n");
					return false;
				}

				DbgPrint("Call destination not in use!\n");

				trampoline_location = operand;
				detour_location = data_pointer;
				return true;

			});

			if (trampoline_location == nullptr)
			{
				DbgPrint("Failed to find possible location, abort!\n");
				return false;
			}

			// SET AS WRITABLE AND EXECUTABLE

			// MAKE PAGE VALID

			if (!physical_memory::protect_memory(trampoline_location, true, true, true))
			{
				DbgPrint("Failed to overwrite memory, abort!\n");
				return false; 
			}

			// GET PAGE INFO
			auto info = physical_memory::query_hardware_table_information(trampoline_location);

			// GET PFN
			auto page_frame_number_pte = MiGetPage(reinterpret_cast<uint32_t*>(MiSystemPartition), 0, 8);
			auto page_frame_number_pde = MiGetPage(reinterpret_cast<uint32_t*>(MiSystemPartition), 0, 8);

			// INITIALIZE PFN
			auto pfn_result_pte = MiInitializePfn(MmPfnDatabase + page_frame_number_pte * 0x30, info.pte, 4, 4);
			auto pfn_result_pde = MiInitializePfn(MmPfnDatabase + page_frame_number_pde * 0x30, info.pde, 4, 4);

			DbgPrint("Pfn result: %02X\n", pfn_result_pte);
			DbgPrint("Pfn result: %02X\n", pfn_result_pde);

			MiMapSinglePage(reinterpret_cast<uint64_t>(trampoline_location), page_frame_number_pte, 0x40000020, 0);

			// UPDATE PDE
			info.pde->u.Hard.PageFrameNumber = page_frame_number_pde;

			// SET EXECUTABLE
			info.pte->u.Hard.NoExecute = 0;
			DbgPrint("Overwrote page protection\n");

			// WRITE TRAMPOLINE WITH OUR DESIRED HOOK AS DESTINATION
			shellcode_generator::call_proxy(trampoline_location, fn);

			// OVERWRITE (DKOM) IOCTL OF DRIVER OBJECT WITH ALLOCATED TRAMPOLINE
			original_handler = *this->major_function(IRP_MJ_DEVICE_CONTROL);

			*this->major_function(IRP_MJ_DEVICE_CONTROL) = reinterpret_cast<PDRIVER_DISPATCH>(detour_location);

			return true;
		}
		// JUST OVERWRITE FUNCTION POINTER
		else
		{
			original_handler = *this->major_function(IRP_MJ_DEVICE_CONTROL);
			*this->major_function(IRP_MJ_DEVICE_CONTROL) = fn;
			return true;
		}
	}

private:
	referenced_object<DRIVER_OBJECT> driver_obj;
};