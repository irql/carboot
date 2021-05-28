
#pragma once

#pragma pack( push, 1 )

typedef union _KGDT_SEGMENT {
    struct {
        ULONG64 LimitPart0 : 16;
        ULONG64 BasePart0 : 16;
        ULONG64 BasePart1 : 8;
        ULONG64 Accessed : 1;
        ULONG64 Write : 1;
        ULONG64 Direction : 1;
        ULONG64 Execute : 1;
        ULONG64 DescriptorType : 1;
        ULONG64 PrivilegeLevel : 2;
        ULONG64 Present : 1;
        ULONG64 LimitPart1 : 4;
        ULONG64 System : 1;
        ULONG64 LongMode : 1;
        ULONG64 DefaultBig : 1;
        ULONG64 Granularity : 1;
        ULONG64 BasePart2 : 8;
    };

    ULONG64 Long;
} KGDT_SEGMENT, *PKGDT_SEGMENT;

typedef union _KGDT_SYSTEM_SEGMENT {

    struct {
        ULONG64 LimitPart0 : 16;
        ULONG64 BasePart0 : 16;
        ULONG64 BasePart1 : 8;
        ULONG64 Type : 4;
        ULONG64 System : 1;
        ULONG64 PrivilegeLevel : 2;
        ULONG64 Present : 1;
        ULONG64 LimitPart1 : 4;
        ULONG64 Avail : 1;
        ULONG64 Reserved1 : 2;
        ULONG64 Granularity : 1;
        ULONG64 BasePart2 : 8;
        ULONG64 BasePart3 : 32;
        ULONG64 Reserved2 : 32;
    };

    struct {
        ULONG64 Long0;
        ULONG64 Long1;
    };
} KGDT_SYSTEM_SEGMENT, *PKGDT_SYSTEM_SEGMENT;

C_ASSERT( sizeof( KGDT_SYSTEM_SEGMENT ) == 16 );

typedef struct _KDESCRIPTOR_TABLE {
    USHORT  Limit;
    ULONG32 Base;
} KDESCRIPTOR_TABLE, *PKDESCRIPTOR_TABLE;

C_ASSERT( sizeof( KDESCRIPTOR_TABLE ) == 6 );

typedef struct _X86_BIOS_INTERRUPT {
    USHORT  SegDs;
    USHORT  SegEs;

    ULONG32 Edi;
    ULONG32 Esi;
    ULONG32 Ebp;
    ULONG32 Esp;
    ULONG32 Ebx;
    ULONG32 Edx;
    ULONG32 Ecx;
    ULONG32 Eax;
} X86_BIOS_INTERRUPT, *PX86_BIOS_INTERRUPT;

C_ASSERT( ( ULONG32 )( &( ( PX86_BIOS_INTERRUPT )0 )->Esp ) == 16 );
C_ASSERT( sizeof( X86_BIOS_INTERRUPT ) );

typedef struct _X86_FAR_PTR {
    USHORT Offset;
    USHORT Segment;
} X86_FAR_PTR, *PX86_FAR_PTR;

typedef struct _X86_VESA_INFO {
    ULONG32     Signature;
    USHORT      Version;
    X86_FAR_PTR OemString;
    ULONG32     Caps;
    X86_FAR_PTR ModeList;
    USHORT      TotalMemory;
    USHORT      OemSoftwareRevision;
    X86_FAR_PTR OemVendorName;
    X86_FAR_PTR OemProductName;
    X86_FAR_PTR OemProductRevision;
    UCHAR       Reserved[ 222 ];
    UCHAR       OemData[ 256 ];
} X86_VESA_INFO, *PX86_VESA_INFO;

C_ASSERT( sizeof( X86_VESA_INFO ) == 512 );

typedef struct _X86_VESA_MODE_INFO {
    USHORT  Attributes;
    UCHAR   WindowA;
    UCHAR   WindowB;
    USHORT  Granularity;
    USHORT  WindowSize;
    USHORT  SegmentA;
    USHORT  SegmentB;
    ULONG32 WindowFunction;
    USHORT  Pitch;
    USHORT  Width;
    USHORT  Height;
    UCHAR   CharWidth;
    UCHAR   CharHeight;
    UCHAR   Planes;
    UCHAR   BitsPerPixel;
    UCHAR   Banks;
    UCHAR   MemoryModel;
    UCHAR   BankSize;
    UCHAR   ImagePages;
    UCHAR   Reserved0;
    UCHAR   RedMask;
    UCHAR   RedPosition;
    UCHAR   GreenMask;
    UCHAR   GreenPosition;
    UCHAR   BlueMask;
    UCHAR   BluePosition;
    UCHAR   AlphaMask;
    UCHAR   AlphaPosition;
    UCHAR   DirectColourAttributes;
    ULONG32 Framebuffer;
    ULONG32 Reserved1;
    USHORT  Reserved2;
    // VBE 3.0
    UCHAR   Reserved3[ 206 ];
} X86_VESA_MODE_INFO;

C_ASSERT( sizeof( X86_VESA_MODE_INFO ) == 256 );

typedef struct _X86_DISK_ACCESS_BLOCK {
    UCHAR   Length;
    UCHAR   Zero;
    USHORT  SectorCount;
    //ULONG32 Buffer;
    X86_FAR_PTR Buffer;
    ULONG64 BlockAddress;
} X86_DISK_ACCESS_BLOCK, *PX86_DISK_ACCESS_BLOCK;

typedef struct _X86_MEMORY_MAP_ENTRY {
    ULONG64 BaseAddress;
    ULONG64 Length;
    ULONG32 Type;
    ULONG32 AcpiEa;
} X86_MEMORY_MAP_ENTRY, *PX86_MEMORY_MAP_ENTRY;

#define X86_BIOS_ENCODE_FAR_PTR( Segment, Offset, Address ) \
Segment = ( ( ULONG32 )( Address ) & 0x000FFFF0 ) >> 4; \
Offset = ( ( ULONG32 )( Address ) & 0x0000000F )

#define X86_BIOS_DECODE_FAR_PTR( Segment, Offset, Address ) \
( ULONG32 )Address = ( ( Segment ) << 4 ) + ( Offset )

VOID
BPAPI
x86BiosCall(
    _In_ ULONG32             Vector,
    _In_ PX86_BIOS_INTERRUPT Interrupt
);

NORETURN
VOID
BPAPI
x86Boot64(
    _In_ ULONG64 EntryAddress,
    _In_ ULONG64 LoaderBlock
);

typedef union _MMPTE_HARDWARE {
    struct {
        ULONG64 Present : 1;
        ULONG64 Write : 1;
        ULONG64 User : 1;
        ULONG64 WriteThrough : 1;
        ULONG64 CacheDisable : 1;
        ULONG64 Accessed : 1;
        ULONG64 Dirty : 1;
        ULONG64 Large : 1;
        ULONG64 Global : 1;
        ULONG64 Available0 : 3;
        ULONG64 PageFrame : 36;
        ULONG64 Reserved1 : 4;
        ULONG64 Available1 : 7;
        ULONG64 ProtectionKey : 4;
        ULONG64 ExecuteDisable : 1;
    } Table;

    struct {
        ULONG64 Present : 1;
        ULONG64 Write : 1;
        ULONG64 User : 1;
        ULONG64 WriteThrough : 1;
        ULONG64 CacheDisable : 1;
        ULONG64 Accessed : 1;
        ULONG64 Dirty : 1;
        ULONG64 PageAttribute : 1;
        ULONG64 Global : 1;
        ULONG64 Available0 : 3;
        ULONG64 PageFrame : 36;
        ULONG64 Reserved1 : 4;
        ULONG64 Available1 : 7;
        ULONG64 ProtectionKey : 4;
        ULONG64 ExecuteDisable : 1;
    } Entry;

    ULONG64     Long;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

C_ASSERT( sizeof( MMPTE_HARDWARE ) == 8 );

#pragma pack( pop )

VOID
BPAPI
BdEnterPagedMode(

);

VOID
BPAPI
BdExitPagedMode(

);

#define IA32_MSR_EFER       0xC0000080

#define CR4_PAE             (1 << 5)

#define CR0_PG              (1 << 31)

#define EFER_LME            (1 << 8)
#define EFER_LMA            (1 << 10)
