
#pragma once

// loader definitions for 64 bit.
// 8 byte packing.

#ifndef C_ASSERT
#define C_ASSERT( x ) static_assert( x, #x )
#endif

#ifndef LOADER_BASIC_TYPES_DEFINED
typedef unsigned long long ULONG64;
typedef unsigned long      ULONG32;
typedef char               CHAR;
#endif

#if defined(_M_AMD64) && (_M_AMD64 == 100)
#define LDR_PTR( TYPE, NAME ) \
TYPE NAME
#else
#define LDR_PTR( TYPE, NAME ) \
union { \
TYPE NAME; \
ULONG64 _##NAME##64; \
}
#endif

#define LOADER_REGION_USABLE        0x00000001
#define LOADER_REGION_RESERVED      0x00000002
#define LOADER_REGION_ACPI_RECLAIM  0x00000003
#define LOADER_REGION_ACPI_NVS      0x00000004
#define LOADER_REGION_BAD           0x00000005
#define LOADER_REGION_LOADER        0x00000006

typedef struct _LOADER_LOGICAL_REGION {
    ULONG64 BaseAddress;
    ULONG64 Length;
    ULONG32 Type;
} LOADER_LOGICAL_REGION, *PLOADER_LOGICAL_REGION;

#define LOADER_REGION_LIST_LENGTH   0x10

typedef struct _LOADER_BOOT_FILE {
    ULONG64 BaseAddress;
    ULONG32 Length;
    CHAR    FileName[ 12 ];
} LOADER_BOOT_FILE, *PLOADER_BOOT_FILE;

#define LOADER_FILE_LIST_LENGTH     0x10

typedef struct _LOADER_SYSTEM_MAP {
    ULONG64 BaseAddress;
    ULONG32 Length;
    LDR_PTR( PLOADER_BOOT_FILE, BootFile );

} LOADER_SYSTEM_MAP, *PLOADER_SYSTEM_MAP;

#define LOADER_MAP_LIST_LENGTH      0x10

typedef struct _LOADER_BOOT_GRAPHICS {
    ULONG32 Width;
    ULONG32 Height;
    ULONG32 Pitch;
    ULONG32 Bpp;
    ULONG32 Frame;
} LOADER_BOOT_GRAPHICS, *PLOADER_BOOT_GRAPHICS;

C_ASSERT( sizeof( LOADER_LOGICAL_REGION ) == 24 );
C_ASSERT( sizeof( LOADER_BOOT_FILE ) == 24 );
C_ASSERT( sizeof( LOADER_SYSTEM_MAP ) == 24 );
C_ASSERT( sizeof( LOADER_BOOT_GRAPHICS ) == 20 );

typedef struct _LOADER_BLOCK {
    ULONG32               LoaderSig;
    ULONG32               RootSerial;

    ULONG32               RegionCount;
    ULONG32               FileCount;
    ULONG32               MapCount;

    LOADER_LOGICAL_REGION RegionList[ LOADER_REGION_LIST_LENGTH ];
    LOADER_BOOT_FILE      FileList[ LOADER_FILE_LIST_LENGTH ];
    LOADER_SYSTEM_MAP     MapList[ LOADER_MAP_LIST_LENGTH ];
    LOADER_BOOT_GRAPHICS  Graphics;

} LOADER_BLOCK, *PLOADER_BLOCK;

C_ASSERT( sizeof( LOADER_BLOCK ) == 1200 );
