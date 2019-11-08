#pragma once
#include <cstdint>

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		} data;
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	struct _ACTIVATION_CONTEXT * EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

// PARTIAL
typedef struct _IMAGE_SECTION_HEADER {
	uint8_t Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		uint32_t   PhysicalAddress;
		uint32_t   VirtualSize;
	} Misc;
	uint32_t VirtualAddress;
	uint32_t SizeOfRawData;
	uint32_t PointerToRawData;
	uint32_t PointerToRelocations;
	uint32_t PointerToLinenumbers;
	uint16_t NumberOfRelocations;
	uint16_t NumberOfLinenumbers;
	uint32_t Characteristics;
	/* STRUCTS */
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

struct IMAGE_DOS_HEADER { // DOS .EXE header
	uint16_t e_magic; // Magic number
	uint16_t e_cblp; // Bytes on last page of file
	uint16_t e_cp; // Pages in file
	uint16_t e_crlc; // Relocations
	uint16_t e_cparhdr; // Size of header in paragraphs
	uint16_t e_minalloc; // Minimum extra paragraphs needed
	uint16_t e_maxalloc; // Maximum extra paragraphs needed
	uint16_t e_ss; // Initial (relative) SS value
	uint16_t e_sp; // Initial SP value
	uint16_t e_csum; // Checksum
	uint16_t e_ip; // Initial IP value
	uint16_t e_cs; // Initial (relative) CS value
	uint16_t e_lfarlc; // File address of relocation table
	uint16_t e_ovno; // Overlay number
	uint16_t e_res[4]; // Reserved words
	uint16_t e_oemid; // OEM identifier (for e_oeminfo)
	uint16_t e_oeminfo; // OEM information; e_oemid specific
	uint16_t e_res2[10]; // Reserved words
	long     e_lfanew; // File address of new exe header
};

struct IMAGE_FILE_HEADER {
	uint16_t      Machine;
	uint16_t      NumberOfSections;
	unsigned long TimeDateStamp;
	unsigned long PointerToSymbolTable;
	unsigned long NumberOfSymbols;
	uint16_t      SizeOfOptionalHeader;
	uint16_t      Characteristics;
};

struct IMAGE_EXPORT_DIRECTORY {
	unsigned long Characteristics;
	unsigned long TimeDateStamp;
	uint16_t      MajorVersion;
	uint16_t      MinorVersion;
	unsigned long Name;
	unsigned long Base;
	unsigned long NumberOfFunctions;
	unsigned long NumberOfNames;
	unsigned long AddressOfFunctions; // RVA from base of image
	unsigned long AddressOfNames; // RVA from base of image
	unsigned long AddressOfNameOrdinals; // RVA from base of image
};

struct IMAGE_DATA_DIRECTORY {
	unsigned long VirtualAddress;
	unsigned long Size;
};

struct IMAGE_OPTIONAL_HEADER64 {
	uint16_t             Magic;
	uint8_t              MajorLinkerVersion;
	uint8_t              MinorLinkerVersion;
	unsigned long        SizeOfCode;
	unsigned long        SizeOfInitializedData;
	unsigned long        SizeOfUninitializedData;
	unsigned long        AddressOfEntryPoint;
	unsigned long        BaseOfCode;
	uint64_t             ImageBase;
	unsigned long        SectionAlignment;
	unsigned long        FileAlignment;
	uint16_t             MajorOperatingSystemVersion;
	uint16_t             MinorOperatingSystemVersion;
	uint16_t             MajorImageVersion;
	uint16_t             MinorImageVersion;
	uint16_t             MajorSubsystemVersion;
	uint16_t             MinorSubsystemVersion;
	unsigned long        Win32VersionValue;
	unsigned long        SizeOfImage;
	unsigned long        SizeOfHeaders;
	unsigned long        CheckSum;
	uint16_t             Subsystem;
	uint16_t             DllCharacteristics;
	uint64_t             SizeOfStackReserve;
	uint64_t             SizeOfStackCommit;
	uint64_t             SizeOfHeapReserve;
	uint64_t             SizeOfHeapCommit;
	unsigned long        LoaderFlags;
	unsigned long        NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];
};

struct IMAGE_NT_HEADERS {
	unsigned long     Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};
