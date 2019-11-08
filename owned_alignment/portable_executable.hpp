#pragma once
#include <cstdint>
#include "nt.hpp"
#include "nt_image.hpp"

struct section_data
{
	uint8_t* start;
	uint64_t size;
};

class portable_executable
{
public:
	portable_executable(uint8_t* address);

	// INFORMATION
	IMAGE_DOS_HEADER* get_dos_header();
	IMAGE_NT_HEADERS* get_nt_headers();
	IMAGE_FILE_HEADER get_file_header();
	IMAGE_OPTIONAL_HEADER64 get_optional_header();
	uintptr_t get_image_base();

	template <SIZE_T N>
	__forceinline section_data get_section(const char(&section_name)[N])
	{
		auto section_pointer = reinterpret_cast<IMAGE_SECTION_HEADER*>(this->nt_headers + 1);

		for (auto index = 0; index < this->file_header.NumberOfSections; index++)
		{
			auto section = &section_pointer[index];

			if (unr::memeq<N>(section->Name, section_name))
			{
				return section_data{
					buffer + section->VirtualAddress,
					section->Misc.VirtualSize
				};
			}
		}

		return section_data{};
	}

private:
	IMAGE_DOS_HEADER* dos_header;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_OPTIONAL_HEADER64 optional_header;
	IMAGE_FILE_HEADER file_header;
	uint8_t* buffer;
};