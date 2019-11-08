#pragma once
#include <cstdint>

// DEFINITIONS
#define VIRTUAL_ADDRESS_BITS 48
#define VIRTUAL_ADDRESS_MASK ((((ULONG_PTR)1) << VIRTUAL_ADDRESS_BITS) - 1)

#define PXE_PER_PAGE 512

#define PXI_MASK (PXE_PER_PAGE - 1)

#define MiGetPxeOffset(va) \
    ((ULONG)(((ULONG_PTR)(va) >> PXI_SHIFT) & PXI_MASK))

#define MiGetPxeAddress(va)   \
    ((PMMPTE)PXE_BASE + MiGetPxeOffset(va))

#define MiGetPpeAddress(va)   \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + PPE_BASE))

#define MiGetPdeAddress(va) \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + PDE_BASE))

#define MiGetPteAddress(va) \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + PTE_BASE))

#define VA_SHIFT (63 - 47)              // address sign extend shift count

#define MiGetVirtualAddressMappedByPte(PTE) \
    ((PVOID)((LONG_PTR)(((LONG_PTR)(PTE) - PTE_BASE) << (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT))

#define MI_IS_PHYSICAL_ADDRESS(Va) \
    ((MiGetPxeAddress(Va)->u.Hard.Valid == 1) && \
     (MiGetPpeAddress(Va)->u.Hard.Valid == 1) && \
     ((MiGetPdeAddress(Va)->u.Long & 0x81) == 0x81) || (MiGetPteAddress(Va)->u.Hard.Valid == 1))

#ifndef PTE_SHIFT
#define PTE_SHIFT 3
#endif
#ifndef PTI_SHIFT
#define PTI_SHIFT 12
#endif
#ifndef PDI_SHIFT
#define PDI_SHIFT 21
#endif
#ifndef PPI_SHIFT
#define PPI_SHIFT 30
#endif
#ifndef PXI_SHIFT
#define PXI_SHIFT 39
#endif

#ifndef PXE_BASE
#define PXE_BASE    0xFFFFF6FB7DBED000UI64
#endif
#ifndef PXE_SELFMAP
#define PXE_SELFMAP 0xFFFFF6FB7DBEDF68UI64
#endif
#ifndef PPE_BASE
#define PPE_BASE    0xFFFFF6FB7DA00000UI64
#endif
#ifndef PDE_BASE
#define PDE_BASE    0xFFFFF6FB40000000UI64
#endif
#ifndef PTE_BASE
#define PTE_BASE    0xFFFFF68000000000UI64
#endif



// STRUCT
typedef struct _MMPTE_SOFTWARE
{
	ULONG Valid : 1;
	ULONG PageFileLow : 4;
	ULONG Protection : 5;
	ULONG Prototype : 1;
	ULONG Transition : 1;
	ULONG Unused : 20;
	ULONG PageFileHigh : 32;
} MMPTE_SOFTWARE, *PMMPTE_SOFTWARE;
typedef struct _HARDWARE_PTE                                                    // 16 / 16 elements; 0x0008 / 0x0008 Bytes
{
	UINT64                      Valid : 1; // ------ / 0x0000; Bit:   0
	UINT64                      Write : 1; // ------ / 0x0000; Bit:   1
	UINT64                      Owner : 1; // ------ / 0x0000; Bit:   2
	UINT64                      WriteThrough : 1; // ------ / 0x0000; Bit:   3
	UINT64                      CacheDisable : 1; // ------ / 0x0000; Bit:   4
	UINT64                      Accessed : 1; // ------ / 0x0000; Bit:   5
	UINT64                      Dirty : 1; // ------ / 0x0000; Bit:   6
	UINT64                      LargePage : 1; // ------ / 0x0000; Bit:   7
	UINT64                      Global : 1; // ------ / 0x0000; Bit:   8
	UINT64                      CopyOnWrite : 1; // ------ / 0x0000; Bit:   9
	UINT64                      Prototype : 1; // ------ / 0x0000; Bit:  10
	UINT64                      reserved0 : 1; // ------ / 0x0000; Bit:  11
	UINT64                      PageFrameNumber : 36; // ------ / 0x0000; Bits: 12 - 47
	UINT64                      reserved1 : 4; // ------ / 0x0000; Bits: 48 - 51
	UINT64                      SoftwareWsIndex : 11; // ------ / 0x0000; Bits: 52 - 62
	UINT64                      NoExecute : 1; // ------ / 0x0000; Bit:  63
} HARDWARE_PTE, *PHARDWARE_PTE;
typedef enum _MI_SYSTEM_VA_TYPE                                                 // 17 / 16 elements; 0x0004 / 0x0004 Bytes
{
	MiVaUnused = 0,
	MiVaSessionSpace = 1,
	MiVaProcessSpace = 2,
	MiVaBootLoaded = 3,
	MiVaPfnDatabase = 4,
	MiVaNonPagedPool = 5,
	MiVaPagedPool = 6,
	MiVaSpecialPoolPaged = 7,
	MiVaSystemCache = 8,
	MiVaSystemPtes = 9,
	MiVaHal = 10,
	MiVaSessionGlobalSpace = 11,
	MiVaDriverImages = 12,
	MiVaSpecialPoolNonPaged = 13,
#if defined(_M_X64)
	MiVaMaximumType = 14,
	MiVaSystemPtesLarge = 15
#else                                                                           // #if defined(_M_X64)
	MiVaPagedProtoPool = 14,
	MiVaMaximumType = 15,
	MiVaSystemPtesLarge = 16
#endif                                                                          // #if defined(_M_X64)
} MI_SYSTEM_VA_TYPE, *PMI_SYSTEM_VA_TYPE;
typedef struct _MMPTE
{
	union
	{
		UINT64                  Long;
		UINT64                  VolatileLong;
		HARDWARE_PTE			Hard;
		MMPTE_SOFTWARE			Soft;
	}u;
} MMPTE, *PMMPTE;
typedef struct _RTL_BITMAP_EX
{
	UINT64                      SizeOfBitMap;
	PUINT64                     Buffer;
} RTL_BITMAP_EX, *PRTL_BITMAP_EX;
typedef struct _MI_SYSTEM_PTE_TYPE
{
	RTL_BITMAP_EX Bitmap;
	PMMPTE                      BasePte;
	ULONG32                     Flags;
	MI_SYSTEM_VA_TYPE           VaType;
	PULONG32                    FailureCount;
	ULONG32                     PteFailures;
	UINT8                       _PADDING0_[4];
	union
	{
		UINT_PTR                SpinLock;
		PULONG_PTR           GlobalPushLock;
	};
	void*						Vm;
	UINT_PTR                    TotalSystemPtes;
	UINT_PTR                    Hint;
	void*						CachedPtes;
	UINT_PTR                    TotalFreeSystemPtes;
} MI_SYSTEM_PTE_TYPE, *PMI_SYSTEM_PTE_TYPE;

#pragma pack(push, 1)
union PML4_BASE {
	uint64_t value;
#pragma warning(disable : 4201)
	struct {
		uint64_t ignored_1 : 3;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t ignored_2 : 7;
		uint64_t pml4_p : 40;
		uint64_t reserved : 12;
	};
};

typedef union VIRT_ADDR_ {
	uint64_t  value;
	void*    pointer;
#pragma warning(disable : 4201)
	struct {
		uint64_t offset : 12;
		uint64_t pt_index : 9;
		uint64_t pd_index : 9;
		uint64_t pdpt_index : 9;
		uint64_t pml4_index : 9;
		uint64_t reserved : 16;
	};
} VIRT_ADDR;

typedef uint64_t PHYS_ADDR;

typedef union PML4E_ {
	uint64_t value;
#pragma warning(disable : 4201)
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t ignored_1 : 1;
		uint64_t reserved_1 : 1;
		uint64_t ignored_2 : 4;
		uint64_t pdpt_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PML4E;

typedef union PDPTE_ {
	uint64_t value;
#pragma warning(disable : 4201)
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t page_size : 1;
		uint64_t ignored_2 : 4;
		uint64_t pd_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PDPTE;

typedef union PDE_ {
	uint64_t value;
#pragma warning(disable : 4201)
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t page_size : 1;
		uint64_t ignored_2 : 4;
		uint64_t pt_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PDE;

typedef union PTE_ {
	uint64_t  value;
	VIRT_ADDR vaddr;
#pragma warning(disable : 4201)
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t pat : 1;
		uint64_t global : 1;
		uint64_t ignored_1 : 3;
		uint64_t page_frame : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PTE;
#pragma pack(pop)