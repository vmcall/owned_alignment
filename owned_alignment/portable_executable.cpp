#include "portable_executable.hpp"

portable_executable::portable_executable(uint8_t* address) : buffer(address)
{
	this->dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(address);
	this->nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(address + this->dos_header->e_lfanew);
	this->file_header = this->nt_headers->FileHeader;
	this->optional_header = this->nt_headers->OptionalHeader;
}

IMAGE_DOS_HEADER* portable_executable::get_dos_header()
{
	return this->dos_header;
}

IMAGE_NT_HEADERS* portable_executable::get_nt_headers()
{
	return this->nt_headers;
}

IMAGE_FILE_HEADER portable_executable::get_file_header()
{
	return this->file_header;
}

IMAGE_OPTIONAL_HEADER64 portable_executable::get_optional_header()
{
	return this->optional_header;
}

uintptr_t portable_executable::get_image_base()
{
	return this->optional_header.ImageBase;
}
