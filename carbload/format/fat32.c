
#include "loader.h"

VOID
BPAPI
BdFat32QueryFatTable(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Index,
    _Out_ ULONG32*            Link
)
{
    ULONG32 FatSector;
    ULONG32 FatSectorIndex;
    ULONG32 FatTable[ 128 ];

    FatSector = Index / 128;
    FatSectorIndex = Index % 128;

    BdDiskRead( &FatTable,
                BootRecord->Bpb3_31.HiddenSectors +
                BootRecord->Bpb2_00.ReservedSectors + FatSector,
                1 );
    *Link = FatTable[ FatSectorIndex ] & 0x0FFFFFFF;
}

VOID
BPAPI
BdFat32ReadCluster(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Number,
    _Out_ PVOID               Buffer
)
{
    ULONG32 Sector;

    Sector = FIRST_SECTOR_OF_CLUSTER( BootRecord, Number );

    BdDiskRead( Buffer,
                BootRecord->Bpb3_31.HiddenSectors +
                Sector,
                BootRecord->Bpb2_00.SectorsPerCluster );
}

ULONG32
BPAPI
BdFat32ReadClusterChain(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             Number,
    _Out_ PVOID               Buffer
)
{
    ULONG32 Length;

    Length = 0;
    do {
        BdFat32ReadCluster( BootRecord,
                            Number,
                            Buffer );
        Buffer = ( PCHAR )Buffer +
            BootRecord->Bpb2_00.SectorsPerCluster * 512;
        Length += BootRecord->Bpb2_00.SectorsPerCluster * 512;

        BdFat32QueryFatTable( BootRecord,
                              Number,
                              &Number );

    } while ( Number != FAT32_END_OF_CHAIN );

    return Length;
}

int memcmp( const void* ptr1, void* ptr2, unsigned long num ) {
    const unsigned char* m1 = ptr1;
    const unsigned char* m2 = ptr2;

    while ( num && *m1 == *m2 )
        if ( --num )
            m1++, m2++;
    return *m1 - *m2;
}

ULONG32
BPAPI
BdFat32ReadDirectoryFile(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  PFAT_DIRECTORY      Directory,
    _In_  PCHAR               Name,
    _Out_ PVOID               Buffer
)
{
    ULONG32 Entry;

    Entry = 0;
    do {

        if ( memcmp( Name, Directory[ Entry ].Short.Name, 11 ) == 0 ) {

            return BdFat32ReadClusterChain( BootRecord,
                                            Directory[ Entry ].Short.ClusterLow |
                                            Directory[ Entry ].Short.ClusterHigh << 16,
                                            Buffer );
        }

        Entry++;
    } while ( Directory[ Entry ].Short.Name[ 0 ] != 0 );

    return 0;
}
