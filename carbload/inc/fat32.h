
#pragma once

#pragma pack( push, 1 )

typedef struct _MBR_PARTITION_ENTRY {
    UCHAR   Attributes;
    UCHAR   StartChs[ 3 ];
    UCHAR   Type;
    UCHAR   EndChs[ 3 ];
    ULONG32 StartLba;
    ULONG32 SectorCount;
} MBR_PARTITION_ENTRY, *PMBR_PARTITION_ENTRY;

typedef struct _MASTER_BOOT_RECORD {
    UCHAR               BootCode[ 440 ];
    ULONG32             UniqueDiskId;
    USHORT              Reserved;
    MBR_PARTITION_ENTRY Table[ 4 ];
    USHORT              BootSignature;
} MASTER_BOOT_RECORD, *PMASTER_BOOT_RECORD;

typedef struct _BPB2_00 {
    CHAR        FileSystemIdentifier[ 8 ];
    USHORT      BytesPerSector;
    UCHAR       SectorsPerCluster;
    USHORT      ReservedSectors;
    UCHAR       FatCount;
    USHORT      RootDirectoryEntriesCount;
    USHORT      TotalSectors16;
    UCHAR       MediaDesciptor;
    USHORT      SectorsPerFat;
} BPB2_00, *PBPB2_00;

typedef struct _BPB3_31 {
    USHORT      SectorsPerTrack;
    USHORT      NumberOfHeads;
    ULONG32     HiddenSectors;
    ULONG32     TotalSectors32;
} BPB3_31, *PBPB3_31;

typedef struct _BPB7_01 {
    ULONG32     SectorsPerFat;
    USHORT      MirroringFlags;
    USHORT      FatVersion;
    ULONG32     RootDirectoryCluster;
    USHORT      FileSystemInfoSector;
    USHORT      BackupSectors;
    UCHAR       Reserved[ 12 ];
    UCHAR       BootDisk;
    UCHAR       NtFlags;
    UCHAR       ExtendedBootSignature;
    ULONG32     SerialNumber;
    UCHAR       VolumeLabel[ 11 ];
    UCHAR       SystemIdentifierString[ 8 ];
} BPB7_01, *PBPB7_01;

typedef struct _VOLUME_BOOT_RECORD {
    CHAR        Jump[ 3 ];
    BPB2_00     Bpb2_00;
    BPB3_31     Bpb3_31;
    BPB7_01     Bpb7_01;
    UCHAR       BootCode[ 420 ];
    USHORT      BootSignature;
} VOLUME_BOOT_RECORD, *PVOLUME_BOOT_RECORD;

#define FAT32_READ_ONLY             0x01
#define FAT32_HIDDEN                0x02
#define FAT32_SYSTEM                0x04
#define FAT32_VOLUME_ID             0x08
#define FAT32_DIRECTORY             0x10
#define FAT32_ARCHIVE               0x20
#define FAT32_LFN                   (FAT32_READ_ONLY | FAT32_HIDDEN | FAT32_SYSTEM | FAT32_VOLUME_ID)

#define FAT32_END_OF_CHAIN          0x0FFFFFFF
#define FAT32_DIRECTORY_ENTRY_FREE  0xE5
#define FAT32_LAST_LFN_ENTRY        0x40

typedef union _FAT_DIRECTORY {
    struct {
        CHAR        Name[ 8 ];
        CHAR        Extension[ 3 ];
        UCHAR       Attributes;
        UCHAR       Reserved;
        UCHAR       CreateTimeTenth;
        USHORT      CreateTime;
        USHORT      CreateDate;
        USHORT      AccessDate;
        USHORT      ClusterHigh;
        USHORT      ModifiedTime;
        USHORT      ModifiedDate;
        USHORT      ClusterLow;
        ULONG32     FileSize;
    } Short;

    struct {
        UCHAR       OrderOfEntry;
        WCHAR       First5Chars[ 5 ];
        UCHAR       Attributes;
        UCHAR       LongEntryType;
        UCHAR       NameChecksum;
        WCHAR       Next6Chars[ 6 ];
        USHORT      Zero;
        WCHAR       Next2Chars[ 2 ];
    } Long;
} FAT_DIRECTORY, *PFAT_DIRECTORY;

#define FIRST_DATA_SECTOR( b )          ( ( b )->Bpb2_00.ReservedSectors + ( ( b )->Bpb2_00.FatCount * ( b )->Bpb7_01.SectorsPerFat ) )
#define FIRST_SECTOR_OF_CLUSTER( b, n ) ( ( ( ( n ) - 2 ) * ( b )->Bpb2_00.SectorsPerCluster ) + FIRST_DATA_SECTOR( b ) )

#pragma pack( pop )
