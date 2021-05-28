
#pragma once

#define KERNEL_FILE_NAME        "KERNEL.SYS"

#define _In_
#define _Out_

#define EXTERN extern
#define STATIC static
#define NORETURN __declspec(noreturn)
#define BPAPI __stdcall
#define FIELD_OFFSET(x, y)      ( ( ULONG32 )&( ( ( x* )0 )->y ) )

#define C_ASSERT( x ) static_assert( x, #x )

#define VOID void
#define NULL 0

#define LOADER_BASIC_TYPES_DEFINED 

typedef unsigned long long ULONG64, *PULONG64;
typedef unsigned long      ULONG32, *PULONG32;
typedef unsigned short     USHORT, *PUSHORT;
typedef unsigned char      UCHAR, *PUCHAR;

typedef long long          LONG64, *PLONG64;
typedef long               LONG32, *PLONG32;
typedef short              SHORT, *PSHORT;
typedef char               CHAR, *PCHAR, *PSTR;

typedef VOID*              PVOID;

typedef short              WCHAR, *PWCHAR;

typedef char*              va_list;

typedef long               NTSTATUS;

#define STATUS_SUCCESS          ( 0x00000000 )
#define STATUS_INVALID_IMAGE    ( 0xC0000000 )
#define STATUS_NOT_FOUND        ( 0xC0000001 )

#define NT_SUCCESS( status )    ( status >= 0 )

#include "pe64.h"
#include "fat32.h"
#include "x86_64.h"

#include "ldef.h"

EXTERN LOADER_BLOCK Loader;
EXTERN ULONG32 BpBootDisk;

#define ROUND_TO_PAGES( Length )    ( ( ( Length ) + 0xFFF ) & ~0xFFF )

VOID
BpDisplayString(
    _In_ PSTR Format,
    _In_ ...
);

NORETURN
VOID
BpFatalException(
    _In_ PSTR Format,
    _In_ ...
);

VOID
BPAPI
BdInitMap(

);

VOID
BPAPI
BdInitGraphics(

);

VOID
BPAPI
BdInitFile(

);

VOID
BPAPI
BdDiskRead(
    _Out_ PVOID   Buffer,
    _In_  ULONG32 BlockAddress,
    _In_  USHORT  SectorCount
);

VOID
BPAPI
BdFat32QueryFatTable(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Index,
    _Out_ ULONG32*            Link
);

VOID
BPAPI
BdFat32ReadCluster(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Number,
    _Out_ PVOID               Buffer
);

ULONG32
BPAPI
BdFat32ReadClusterChain(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Number,
    _Out_ PVOID               Buffer
);

ULONG32
BPAPI
BdFat32ReadDirectoryFile(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  PFAT_DIRECTORY      Directory,
    _In_  PCHAR               Name,
    _Out_ PVOID               Buffer
);

ULONG64
BPAPI
BdAddress64(
    _In_ ULONG32 Address32
);

VOID
BPAPI
BdTranslateLoader(

);

NTSTATUS
LdrGetExportAddressByName(
    _In_  PVOID    Base,
    _In_  PCHAR    Name,
    _Out_ ULONG64* Address
);

NTSTATUS
LdrGetExportAddressByOrdinal(
    _In_  PVOID    Base,
    _In_  USHORT   Ordinal,
    _Out_ ULONG64* Address
);

NTSTATUS
LdrResolveBaseReloc(
    _In_ ULONG32 BaseAddress
);

NTSTATUS
LdrResolveImportTable(
    _In_ PVOID                    Importer,
    _In_ PVOID                    Importee,
    _In_ PIMAGE_IMPORT_DESCRIPTOR Import
);

NTSTATUS
LdrLoadSystemModule(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  PLOADER_BOOT_FILE   BootFile,
    _In_  ULONG32             BaseAddress
);

ULONG32
LdrEntryPoint(
    _In_ ULONG32 BaseAddress
);

void  __cdecl __va_start( va_list*, ... );
void* __cdecl __va_arg( va_list*, ... );
void  __cdecl __va_end( va_list* );

#define _ADDRESSOF(v) (&(v))
#define _SLOTSIZEOF(t)  (sizeof(t))
#define _APALIGN(t,ap)  (__alignof(t))

#define __crt_va_start(ap, v)   ((void)(__va_start(&ap, _ADDRESSOF(v), _SLOTSIZEOF(v), __alignof(v), _ADDRESSOF(v))))
#define __crt_va_arg(ap, t)     (*(t *)__va_arg(&ap, _SLOTSIZEOF(t), _APALIGN(t,ap), (t*)0))
#define __crt_va_end(ap)        ((void)(__va_end(&ap)))

VOID
BPAPI
RtlFormatAnsiStringFromList(
    _In_ PSTR    Buffer,
    _In_ PSTR    Format,
    _In_ va_list List
);

VOID
RtlFormatAnsiString(
    _In_ PSTR Buffer,
    _In_ PSTR Format,
    _In_ ...
);

LONG32
RtlCompareAnsiString(
    _In_ PSTR  String1,
    _In_ PSTR  String2,
    _In_ UCHAR CaseInsensitive
);

VOID
RtlFillMemory(
    _In_ PVOID   Destination,
    _In_ ULONG32 Fill,
    _In_ ULONG32 Length
);

VOID
RtlCopyMemory(
    _In_ PVOID   Destination,
    _In_ PVOID   Source,
    _In_ ULONG32 Length
);

#define RtlUpperChar( c )               ( ( c ) >= 'a' && ( c ) <= 'z' ? ( c ) - ' ' : ( c ) )
#define RtlLowerChar( c )               ( ( c ) >= 'A' && ( c ) <= 'Z' ? ( c ) + ' ' : ( c ) )

VOID
BPAPI
BdFindSystemFile(
    _In_  PSTR                Name,
    _Out_ PLOADER_SYSTEM_MAP* Map
);

VOID
BPAPI
BdFindBootFile(
    _In_  PSTR               Name,
    _Out_ PLOADER_BOOT_FILE* BootFile
);
