#include "physical_memory.hpp"

hardware_pt_info physical_memory::query_hardware_table_information(const void* virtual_address)
{
	hardware_pt_info result{};

	// THIS IS ONLY FOR SUB RS2 WINDOWS VERSIONS, THANKS TO EXPLOIT MITIGATION 

	auto temp_pde = MiGetPdeAddress(virtual_address);

	if (!MI_IS_PHYSICAL_ADDRESS(temp_pde))
		return result;

	result.pde = temp_pde;

	if (result.pde->u.Hard.LargePage)
	{
		result.is_large_page = true;
		return result;
	}

	auto temp_pte = MiGetPteAddress(virtual_address);

	if (!MI_IS_PHYSICAL_ADDRESS(temp_pte))
		return result;

	result.pte = temp_pte;

	return result;
}

bool physical_memory::protect_memory(const void* virtual_address, bool write, bool execute, bool fix_invalid)
{
	auto page_info = physical_memory::query_hardware_table_information(virtual_address);
	bool refetch = false;

	DbgPrint("Fetched page info\n");

	if (page_info.pde == nullptr)
	{
		DbgPrint("Invalid PDE!\n");
		return false;
	}

	// RECONSTRUCT TABLE ENTRY?
	if (fix_invalid)
	{
		DbgPrint("Fixing invalid\n");
		if (page_info.pde->u.Hard.Valid == 0)
		{
			DbgPrint("Invalid PDE. Fixing...\n");
			page_info.pde->u.Hard.Dirty = 1;
			page_info.pde->u.Hard.Accessed = 1;
			page_info.pde->u.Hard.Owner = 0;
			page_info.pde->u.Hard.Write = 1;
			page_info.pde->u.Hard.NoExecute = 0;
			page_info.pde->u.Hard.Valid = 1;
			refetch = true;
		}
	}

	// REFETCH AFTER PDE RECONSTRUCTION TO GET PROPER PTE
	if (refetch)
	{
		page_info = physical_memory::query_hardware_table_information(virtual_address);
	}

	// NO PTE?
	if (page_info.pte == nullptr || !physical_memory::is_valid_address(reinterpret_cast<uint8_t*>(page_info.pte)))
	{
		DbgPrint("Lookup of PTE failed!\n");
		return false;
	}
	// VALID PTE
	else if (page_info.pte->u.Hard.Valid == 1)
	{
		DbgPrint("Found valid page!\n");
		page_info.pte->u.Hard.NoExecute = execute ? 0ull : 1ull;
		page_info.pte->u.Hard.Write = write ? 1ull : 0ull;
		return true;
	}
	// FIX INVALID PAGE AND SET PROTECTION
	else if (fix_invalid && page_info.pte->u.Hard.Valid == 0)
	{
		DbgPrint("Patching PTE.\n");

		*page_info.pte = MMPTE{};
	
		page_info.pte->u.Hard.Dirty = 1;
		page_info.pte->u.Hard.Accessed = 1;
		page_info.pte->u.Hard.Owner = 0;
		page_info.pte->u.Hard.Write = write ? 1ull : 0ull;
		page_info.pte->u.Hard.NoExecute = execute ? 0ull : 1ull;
		page_info.pte->u.Hard.Valid = 1;
		return true;
	}

	// SHOULD NEVER HAPPEN
	DbgPrint("UNEXPECTED Lookup of PTE failed!\n");
	return false;
}

bool physical_memory::is_valid_address(const void* virtual_address)
{
	if (virtual_address == nullptr)
		return false;

	return MmIsAddressValid(const_cast<void*>(virtual_address));
}

