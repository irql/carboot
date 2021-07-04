
#include <loader.h>

VOID
BPAPI
BdLoadBootFile(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  ULONG32             BaseAddress,
    _In_  PFAT_DIRECTORY      Directory,
    _In_  PSTR                FileName,
    _Out_ PLOADER_BOOT_FILE   BootFile
)
{
    BootFile->BaseAddress = BaseAddress;
    BootFile->Length = BdFat32ReadDirectoryFile( BootRecord,
                                                 Directory,
                                                 FileName,
                                                 ( PVOID )BaseAddress );
    RtlCopyMemory( BootFile->FileName, FileName, 11 );
    BootFile->FileName[ 11 ] = 0;
}

VOID
BPAPI
BdFatName(
    _In_  PSTR Name,
    _Out_ PSTR FatName
)
{
    ULONG32 Index;

    RtlFillMemory( FatName, ' ', 11 );
    FatName[ 11 ] = 0;

    for ( Index = 0;
          Index < 11 &&
          Name[ Index ] != '.' &&
          Name[ Index ] != 0;
          Index++ ) {

        FatName[ Index ] = RtlUpperChar( Name[ Index ] );
    }

    if ( Name[ Index ] == '.' ) {

        FatName[ 8 ] = RtlUpperChar( Name[ Index + 1 ] );
        FatName[ 9 ] = RtlUpperChar( Name[ Index + 2 ] );
        FatName[ 10 ] = RtlUpperChar( Name[ Index + 3 ] );
    }
}

VOID
BPAPI
BdFindSystemFile(
    _In_  PSTR                Name,
    _Out_ PLOADER_SYSTEM_MAP* Map
)
{
    ULONG32 CurrentMap;
    CHAR    FileName[ 12 ];

    BdFatName( Name, FileName );

    *Map = NULL;

    for ( CurrentMap = 0; CurrentMap < Loader.MapCount; CurrentMap++ ) {

        if ( Loader.MapList[ CurrentMap ].BootFile == NULL ) {

            continue;
        }

        if ( RtlCompareAnsiString( FileName,
                                   Loader.MapList[ CurrentMap ].BootFile->FileName,
                                   1 ) == 0 ) {

            *Map = &Loader.MapList[ CurrentMap ];
            return;
        }
    }
}

VOID
BPAPI
BdFindBootFile(
    _In_  PSTR               Name,
    _Out_ PLOADER_BOOT_FILE* BootFile
)
{
    ULONG32 CurrentFile;
    CHAR    FileName[ 12 ];

    BdFatName( Name, FileName );

    *BootFile = NULL;

    for ( CurrentFile = 0; CurrentFile < Loader.FileCount; CurrentFile++ ) {

        if ( RtlCompareAnsiString( FileName, Loader.FileList[ CurrentFile ].FileName, 1 ) == 0 ) {

            *BootFile = &Loader.FileList[ CurrentFile ];
            return;
        }
    }
}

VOID
BPAPI
BdInitFile(

)
{
    PVOLUME_BOOT_RECORD BootRecord = ( PVOLUME_BOOT_RECORD )0x7C00;
    PFAT_DIRECTORY      RootDirectory = ( PFAT_DIRECTORY )0x200000;
    PFAT_DIRECTORY      CurrentDirectory;
    ULONG32             Length;
    ULONG32             CurrentFile;
    ULONG32             CurrentBase;
    CHAR                FatName[ 12 ];

    BpBootDisk = BootRecord->Bpb7_01.BootDisk;

    Loader.LoaderSig = 'BRAC';
    Loader.RootSerial = BootRecord->Bpb7_01.SerialNumber;

    BdFat32ReadCluster( BootRecord,
                        2,
                        RootDirectory );

    CurrentDirectory = RootDirectory + BootRecord->Bpb2_00.SectorsPerCluster * 512;
    CurrentBase = ( ULONG32 )CurrentDirectory;
    Length = BdFat32ReadDirectoryFile( BootRecord,
                                       RootDirectory,
                                       "SYSTEM     ",
                                       CurrentDirectory );
    if ( Length == 0 ) {

        BpFatalException( "FATAL: failed to find SYSTEM directory." );
    }

    BdFatName( KERNEL_FILE_NAME, FatName );

    CurrentBase += Length;
    BdLoadBootFile( BootRecord,
                    CurrentBase,
                    CurrentDirectory,
                    FatName,
                    &Loader.FileList[ Loader.FileCount ] );
    if ( Loader.FileList[ Loader.FileCount ].Length == 0 ) {

        BpFatalException( "FATAL: failed to find kernel module." );
    }

    CurrentBase += Loader.FileList[ Loader.FileCount++ ].Length;

    Length = BdFat32ReadDirectoryFile( BootRecord,
                                       CurrentDirectory,
                                       "BOOT       ",
                                       CurrentDirectory );

    if ( Length == 0 ) {

        BpFatalException( "FATAL: failed to find BOOT directory." );
    }

    for ( CurrentFile = 0;
          CurrentDirectory[ CurrentFile ].Short.Name[ 0 ] != 0;
          CurrentFile++ ) {

        if ( ( UCHAR )CurrentDirectory[ CurrentFile ].Short.Name[ 0 ] == ( UCHAR )0xE5 ) {

            continue;
        }

        if ( ( UCHAR )CurrentDirectory[ CurrentFile ].Short.Attributes & FAT32_DIRECTORY ||
            ( UCHAR )CurrentDirectory[ CurrentFile ].Short.Attributes & FAT32_VOLUME_ID ||
             ( ( UCHAR )CurrentDirectory[ CurrentFile ].Short.Attributes & FAT32_ARCHIVE ) == 0 ) {

            continue;
        }

        Loader.FileList[ Loader.FileCount ].BaseAddress = CurrentBase;
        Loader.FileList[ Loader.FileCount ].Length =
            BdFat32ReadClusterChain( BootRecord,
                                     CurrentDirectory[ CurrentFile ].Short.ClusterLow |
                                     CurrentDirectory[ CurrentFile ].Short.ClusterHigh << 16,
                                     ( PVOID )CurrentBase );
        RtlCopyMemory( Loader.FileList[ Loader.FileCount ].FileName,
                       CurrentDirectory[ CurrentFile ].Short.Name,
                       11 );
        Loader.FileList[ Loader.FileCount ].FileName[ 11 ] = 0;

        CurrentBase += Loader.FileList[ Loader.FileCount ].Length;
        /*
        BpDisplayString( "BdInitFile: %s %llx %lx\n",
                         Loader.FileList[ Loader.FileCount ].FileName,
                         Loader.FileList[ Loader.FileCount ].BaseAddress,
                         Loader.FileList[ Loader.FileCount ].Length );
                         */
        Loader.FileCount++;
    }
}
