
#include "loader.h"

VOID
BPAPI
BdDiskRead(
    _Out_ PVOID   Buffer,
    _In_  ULONG32 BlockAddress,
    _In_  USHORT  SectorCount
)
{
    //
    // TODO: Must re-read over & over if the buffer is
    // outside the segment, this function should deal
    // with that.
    //

    X86_DISK_ACCESS_BLOCK Access = { 0 };
    X86_BIOS_INTERRUPT BiosFrame = { 0 };
    PCHAR Low = ( PCHAR )0x10000;

    Access.Length = 0x10;
    Access.Zero = 0;
    X86_BIOS_ENCODE_FAR_PTR( Access.Buffer.Segment,
                             Access.Buffer.Offset,
                             Low );
    Access.BlockAddress = BlockAddress;
    Access.SectorCount = SectorCount;

    BiosFrame.Eax = 0x4200;
    BiosFrame.Edx = BpBootDisk;
    X86_BIOS_ENCODE_FAR_PTR( BiosFrame.SegDs,
                             BiosFrame.Esi,
                             &Access );
    x86BiosCall( 0x13, &BiosFrame );

    __movsb( Buffer, Low, SectorCount * 512 );
}
