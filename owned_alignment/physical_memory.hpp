#pragma once
#include "nt.hpp"
#include <cstdint>
#include "nt_pagetable.hpp"

struct page_table_info {
    PML4E* pml4_entry;
    PDPTE* pdpt_entry;
    PDE*   pd_entry;
    PTE*   pt_entry;
};

struct hardware_pt_info
{
	MMPTE* pde;
	MMPTE* pte;
	bool is_large_page;
};

namespace physical_memory {

	hardware_pt_info query_hardware_table_information(const void* virtual_address);

	bool protect_memory(const void* virtual_address, bool write, bool execute, bool fix_invalid);

	bool is_valid_address(const void* virtual_address);

    template<class Fn>
    void iterate_region(const uint8_t* start, const uint64_t size, Fn fn)
    {
        // ITERATE PHYSICAL MEMORY REGION (IN ALIGNED CHUNKS)
        const auto address_end  = start + size;
        auto       address_iter = start;

        while(address_iter < address_end) {
            // CALCULATE DEFAULT PAGE SIZE
            auto page_size =
                reinterpret_cast<uint8_t*>(
                    (reinterpret_cast<uint64_t>(address_iter) + 0x1000) & (~0xFFF)) -
                address_iter;

            // IF DEFAULT PAGE SIZE EXCEEDS END, TRUNCATE SIZE
            if(address_iter + page_size > address_end)
                page_size = address_end - address_iter;

            // CALL RESPESCTIVE HANDLER FUNCTION
            fn(address_iter, page_size);

            // ADD SIZE TO POINTER
            address_iter += page_size;
        }
    }
} // namespace physical_memory