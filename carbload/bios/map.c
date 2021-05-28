
#include "loader.h"

VOID
BdReserveRegion(
    _In_ ULONG32 LogicalAddress,
    _In_ ULONG32 Length
)
{
    ULONG32              CurrentRegion;

    for ( CurrentRegion = 0; CurrentRegion < Loader.RegionCount; CurrentRegion++ ) {

        if ( Loader.RegionList[ CurrentRegion ].Type != LOADER_REGION_USABLE ||
             Loader.RegionList[ CurrentRegion ].BaseAddress >= LogicalAddress + Length ||
             Loader.RegionList[ CurrentRegion ].BaseAddress +
             Loader.RegionList[ CurrentRegion ].Length <= LogicalAddress + Length ) {

            continue;
        }

        if ( Loader.RegionList[ CurrentRegion ].BaseAddress != LogicalAddress ) {

            Loader.RegionList[ Loader.RegionCount ].BaseAddress =
                Loader.RegionList[ CurrentRegion ].BaseAddress;
            Loader.RegionList[ Loader.RegionCount ].Length =
                LogicalAddress - Loader.RegionList[ CurrentRegion ].BaseAddress;
            Loader.RegionList[ Loader.RegionCount ].Type = LOADER_REGION_USABLE;

            Loader.RegionList[ CurrentRegion ].BaseAddress +=
                Loader.RegionList[ Loader.RegionCount ].Length;
            Loader.RegionList[ CurrentRegion ].Length -=
                Loader.RegionList[ Loader.RegionCount ].Length;

            Loader.RegionCount++;

        }

        Loader.RegionList[ Loader.RegionCount ].BaseAddress =
            LogicalAddress;
        Loader.RegionList[ Loader.RegionCount ].Length =
            LogicalAddress + Length - Loader.RegionList[ CurrentRegion ].BaseAddress;
        Loader.RegionList[ Loader.RegionCount ].Type = LOADER_REGION_LOADER;

        Loader.RegionList[ CurrentRegion ].BaseAddress +=
            Loader.RegionList[ Loader.RegionCount ].Length;
        Loader.RegionList[ CurrentRegion ].Length -=
            Loader.RegionList[ Loader.RegionCount ].Length;

        Loader.RegionCount++;
    }
}

ULONG32
BdClosestLogical(
    _In_ ULONG32 LogicalAddress,
    _In_ ULONG32 MinimumLength
)
{
    ULONG32              CurrentRegion;
    ULONG64              ClosestLogical;

    ClosestLogical = 0;
    for ( CurrentRegion = 0; CurrentRegion < Loader.RegionCount; CurrentRegion++ ) {

        if ( Loader.RegionList[ CurrentRegion ].Type != LOADER_REGION_USABLE ||
             Loader.RegionList[ CurrentRegion ].BaseAddress >= LogicalAddress ||
             Loader.RegionList[ CurrentRegion ].BaseAddress +
             Loader.RegionList[ CurrentRegion ].Length <= LogicalAddress ||
             Loader.RegionList[ CurrentRegion ].BaseAddress -
             LogicalAddress +
             Loader.RegionList[ CurrentRegion ].Length < MinimumLength ) {

            continue;
        }

        return LogicalAddress;
    }

    for ( CurrentRegion = 0; CurrentRegion < Loader.RegionCount; CurrentRegion++ ) {

        if ( Loader.RegionList[ CurrentRegion ].Type != LOADER_REGION_USABLE ||
             Loader.RegionList[ CurrentRegion ].Length < MinimumLength ) {

            continue;
        }

        if ( Loader.RegionList[ CurrentRegion ].BaseAddress - LogicalAddress <
             ClosestLogical - LogicalAddress ) {

            ClosestLogical = Loader.RegionList[ CurrentRegion ].BaseAddress;
        }
    }

    return ClosestLogical;
}

ULONG32 PagedMode = 0;

VOID
BPAPI
BdEnterPagedMode(

)
{
    if ( PagedMode == 1 ) {
        __writecr4( __readcr4( ) | CR4_PAE );
        __writemsr( IA32_MSR_EFER, __readmsr( IA32_MSR_EFER ) | EFER_LME );
        __writecr0( __readcr0( ) | CR0_PG );
        PagedMode = 2;
    }
}

VOID
BPAPI
BdExitPagedMode(

)
{
    if ( PagedMode == 2 ) {
        //__writemsr( IA32_MSR_EFER, __readmsr( IA32_MSR_EFER ) & ~EFER_LMA );
        __writecr0( __readcr0( ) & ~CR0_PG );
        __writecr4( __readcr4( ) & ~CR4_PAE );
        __writemsr( IA32_MSR_EFER, __readmsr( IA32_MSR_EFER ) & ~EFER_LME );
        PagedMode = 1;
    }
}

VOID
BPAPI
BdInitMap(

)
{
    X86_BIOS_INTERRUPT   BiosFrame = { 0 };
    X86_MEMORY_MAP_ENTRY MapEntry;
    ULONG32              CurrentRegion;
    PMMPTE_HARDWARE      Pxe;
    PMMPTE_HARDWARE      PpeLower;
    PMMPTE_HARDWARE      PpeUpper;
    PMMPTE_HARDWARE      PdeLower;
    PMMPTE_HARDWARE      PdeUpper;
    ULONG32              BaseReserve;

    BiosFrame.SegDs = 0;
    BiosFrame.Ebx = 0;
    do {

        BiosFrame.Eax = 0xE820;
        BiosFrame.Edx = 'SMAP';
        BiosFrame.Ecx = sizeof( X86_MEMORY_MAP_ENTRY );
        X86_BIOS_ENCODE_FAR_PTR( BiosFrame.SegEs,
                                 BiosFrame.Edi,
                                 &MapEntry );
        x86BiosCall( 0x15, &BiosFrame );

        Loader.RegionList[ Loader.RegionCount ].BaseAddress = MapEntry.BaseAddress;
        Loader.RegionList[ Loader.RegionCount ].Length = MapEntry.Length;
        Loader.RegionList[ Loader.RegionCount ].Type = MapEntry.Type;

        Loader.RegionCount++;
    } while ( BiosFrame.Ebx != 0 );

    BdReserveRegion( 0, 0x100000 );
    // could cause an issue if it is outside the 0x200000 range.
    BaseReserve = BdClosestLogical( 0x100000, 0x5000 );
    BdReserveRegion( BaseReserve, 0x5000 );

    Pxe = ( PMMPTE_HARDWARE )BaseReserve;
    PpeLower = ( PMMPTE_HARDWARE )( BaseReserve + 0x1000 );
    PpeUpper = ( PMMPTE_HARDWARE )( BaseReserve + 0x2000 );
    PdeLower = ( PMMPTE_HARDWARE )( BaseReserve + 0x3000 );
    PdeUpper = ( PMMPTE_HARDWARE )( BaseReserve + 0x4000 );
    __stosb( Pxe, 0, 0x1000 );
    __stosb( PpeLower, 0, 0x1000 );
    __stosb( PpeUpper, 0, 0x1000 );
    __stosb( PdeLower, 0, 0x1000 );
    __stosb( PdeUpper, 0, 0x1000 );

    Pxe[ 0 ].Table.Present = 1;
    Pxe[ 0 ].Table.Write = 1;
    Pxe[ 0 ].Table.PageFrame = ( ULONG64 )( ULONG32 )PpeLower >> 12;

    Pxe[ 511 ].Table.Present = 1;
    Pxe[ 511 ].Table.Write = 1;
    Pxe[ 511 ].Table.PageFrame = ( ULONG64 )( ULONG32 )PpeUpper >> 12;

    PpeLower[ 0 ].Table.Present = 1;
    PpeLower[ 0 ].Table.Write = 1;
    PpeLower[ 0 ].Table.PageFrame = ( ULONG64 )( ULONG32 )PdeLower >> 12;

    PpeUpper[ 511 ].Table.Present = 1;
    PpeUpper[ 511 ].Table.Write = 1;
    PpeUpper[ 511 ].Table.PageFrame = ( ULONG64 )( ULONG32 )PdeUpper >> 12;

    BaseReserve = BdClosestLogical( 0x200000, 0x200000 );

    PdeLower[ 0 ].Table.Present = 1;
    PdeLower[ 0 ].Table.Write = 1;
    PdeLower[ 0 ].Table.Large = 1;
    PdeLower[ 0 ].Table.PageFrame = 0;

    PdeLower[ 1 ].Table.Present = 1;
    PdeLower[ 1 ].Table.Write = 1;
    PdeLower[ 1 ].Table.Large = 1;
    PdeLower[ 1 ].Table.PageFrame = ( ULONG64 )( ULONG32 )BaseReserve >> 12;

    PdeUpper[ 510 ].Table.Present = 1;
    PdeUpper[ 510 ].Table.Write = 1;
    PdeUpper[ 510 ].Table.Large = 1;
    PdeUpper[ 510 ].Table.PageFrame = 0;

    PdeUpper[ 511 ].Table.Present = 1;
    PdeUpper[ 511 ].Table.Write = 1;
    PdeUpper[ 511 ].Table.Large = 1;
    PdeUpper[ 511 ].Table.PageFrame = ( ULONG64 )( ULONG32 )BaseReserve >> 12;

    BdReserveRegion( BaseReserve, 0x200000 );

    __writecr3( Pxe );
    PagedMode = 1;
    BdEnterPagedMode( );
}

ULONG64
BPAPI
BdAddress64(
    _In_ ULONG32 Address32
)
{
    return ( ULONG64 )Address32 + 0xFFFFFFFFFFC00000;
}

VOID
BPAPI
BdTranslateLoader(

)
{
    ULONG32 CurrentEntry;

    for ( CurrentEntry = 0; CurrentEntry < Loader.FileCount; CurrentEntry++ ) {

        Loader.FileList[ CurrentEntry ].BaseAddress =
            BdAddress64( Loader.FileList[ CurrentEntry ].BaseAddress );
    }

    for ( CurrentEntry = 0; CurrentEntry < Loader.MapCount; CurrentEntry++ ) {

        Loader.MapList[ CurrentEntry ].BaseAddress =
            BdAddress64( Loader.MapList[ CurrentEntry ].BaseAddress );

        Loader.MapList[ CurrentEntry ]._BootFile64 =
            BdAddress64( ( ULONG32 )Loader.MapList[ CurrentEntry ].BootFile );
    }
}
