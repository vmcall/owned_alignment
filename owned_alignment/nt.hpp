#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <ntdddisk.h>
#include <intrin.h>
#include <cstdint>

/* MACROS / DEFINITIONS */
using UINT = unsigned int;
//using ULONG = unsigned int;
using DWORD = unsigned int;
using WORD = __int16;
using BYTE = __int8;
using BOOL = bool;

#define PFN_TO_PAGE(pfn) (pfn << 12)
#define PAGE_TO_PFN(page) (page >> 12)
#define IMAGE_SIZEOF_SHORT_NAME              8



