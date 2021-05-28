
#include <loader.h>

#pragma section( ".LDRBLCK", read, write )

__declspec( allocate( ".LDRBLCK" ) ) LOADER_BLOCK Loader = { 0 };

ULONG32 BpBootDisk;

#define GDT_SEGMENT_R0_CODE32 0x08
#define GDT_SEGMENT_R0_DATA32 0x10
#define GDT_SEGMENT_R0_CODE16 0x18
#define GDT_SEGMENT_R0_DATA16 0x20
#define GDT_SEGMENT_R0_CODE64 0x28
#define GDT_SEGMENT_R0_DATA64 0x30
#define GDT_SEGMENT_COUNT     7

KGDT_SEGMENT SegmentTable[ GDT_SEGMENT_COUNT ] = {
    { 0 },

    //GDT_SEGMENT_R0_CODE32
    {
        .LimitPart0 = 0xFFFF,
        .LimitPart1 = 0x0F,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 1,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 0,
        .DefaultBig = 1,
        .Granularity = 1,
    },

    //GDT_SEGMENT_R0_DATA32
    {
        .LimitPart0 = 0xFFFF,
        .LimitPart1 = 0x0F,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 0,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 0,
        .DefaultBig = 1,
        .Granularity = 1,
    },

    //GDT_SEGMENT_R0_CODE16
    {
        .LimitPart0 = 0xFFFF,
        .LimitPart1 = 0,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 1,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 0,
        .DefaultBig = 0,
        .Granularity = 0,
    },

    //GDT_SEGMENT_R0_DATA16
    {
        .LimitPart0 = 0xFFFF,
        .LimitPart1 = 0,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 0,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 0,
        .DefaultBig = 0,
        .Granularity = 0,
    },

    //GDT_SEGMENT_R0_CODE64
    {
        .LimitPart0 = 0xFFFF,
        .LimitPart1 = 0xF,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 1,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 1,
        .DefaultBig = 0,
        .Granularity = 1,
    },

    //GDT_SEGMENT_R0_DATA64
    {
        .LimitPart0 = 0,
        .LimitPart1 = 0,
        .BasePart0 = 0,
        .BasePart1 = 0,
        .BasePart2 = 0,

        .Accessed = 0,
        .Write = 1,
        .Direction = 0,
        .Execute = 0,
        .DescriptorType = 1,
        .PrivilegeLevel = 0,
        .Present = 1,

        .System = 0,
        .LongMode = 0,
        .DefaultBig = 0,
        .Granularity = 0,
    },
};

VOID
BPAPI
BpDisplayRawString(
    _In_ PSTR String
)
{
    STATIC ULONG32 PosX = 0;
    STATIC ULONG32 PosY = 0;

    while ( *String ) {

        switch ( *String ) {
        case '\n':
            PosY++;
            PosX = 0;
            break;
        default:
            ( ( USHORT* )0xB8000 )[ PosX + PosY * 80 ] = *String | 0x0F00;
            PosX++;
            break;
        }
        String++;
    }
}

VOID
BpDisplayString(
    _In_ PSTR Format,
    _In_ ...
)
{
    va_list List;
    CHAR    Buffer[ 512 ];

    __crt_va_start( List, Format );
    RtlFormatAnsiStringFromList( Buffer,
                                 Format,
                                 List );
    __crt_va_end( List );
    BpDisplayRawString( Buffer );
}

NORETURN
VOID
BpFatalException(
    _In_ PSTR Format,
    _In_ ...
)
{
    va_list List;
    CHAR    Buffer[ 512 ];
    //X86_BIOS_INTERRUPT BiosFrame = { 0 };

    __crt_va_start( List, Format );
    RtlFormatAnsiStringFromList( Buffer,
                                 Format,
                                 List );
    __crt_va_end( List );

    //BiosFrame.Eax = 0x83;
    //x86BiosCall( 0x10, &BiosFrame );

    BpDisplayRawString( Buffer );
    __halt( );
}

VOID
BPAPI
BdDisplayLoader(

)
{
    ULONG32 CurrentEntry;
    CHAR    LoaderSig[ 5 ];

    *( ULONG32* )LoaderSig = Loader.LoaderSig;
    LoaderSig[ 4 ] = 0;

    BpDisplayString( "LOADERSIG: %s\n", LoaderSig );
    BpDisplayString( "ROOTSERIAL: %lx\n", Loader.RootSerial );

    for ( CurrentEntry = 0; CurrentEntry < Loader.RegionCount; CurrentEntry++ ) {

        BpDisplayString( "REGION %l: %llx %llx %l\n",
                         CurrentEntry,
                         Loader.RegionList[ CurrentEntry ].BaseAddress,
                         Loader.RegionList[ CurrentEntry ].Length,
                         Loader.RegionList[ CurrentEntry ].Type );
    }

    for ( CurrentEntry = 0; CurrentEntry < Loader.FileCount; CurrentEntry++ ) {

        BpDisplayString( "FILE %l: %s %llx %lx\n",
                         CurrentEntry,
                         Loader.FileList[ CurrentEntry ].FileName,
                         BdAddress64( Loader.FileList[ CurrentEntry ].BaseAddress ),
                         Loader.FileList[ CurrentEntry ].Length );
    }

    for ( CurrentEntry = 0; CurrentEntry < Loader.MapCount; CurrentEntry++ ) {

        if ( Loader.MapList[ CurrentEntry ].BootFile != NULL &&
            ( Loader.MapList[ CurrentEntry ]._BootFile64 >> 32 ) == NULL ) {
            BpDisplayString( "MAP %l: %s: %llx %lx\n",
                             CurrentEntry,
                             Loader.MapList[ CurrentEntry ].BootFile->FileName,
                             BdAddress64( Loader.MapList[ CurrentEntry ].BaseAddress ),
                             Loader.MapList[ CurrentEntry ].Length );
        }
        else {
            BpDisplayString( "MAP %l: %llx %lx\n",
                             CurrentEntry,
                             BdAddress64( Loader.MapList[ CurrentEntry ].BaseAddress ),
                             Loader.MapList[ CurrentEntry ].Length );
        }
    }

    BpDisplayString( "GRAPHICS: %l,%l,%l,%lx\n",
                     Loader.Graphics.Width,
                     Loader.Graphics.Height,
                     Loader.Graphics.Bpp,
                     Loader.Graphics.Frame );
}

VOID
BPAPI
BpLoadSystem(

)
{
    KDESCRIPTOR_TABLE GlobalDescriptor = {
        sizeof( KGDT_SEGMENT[ GDT_SEGMENT_COUNT ] ) - 1,
        ( ULONG32 )&SegmentTable
    };

    PVOLUME_BOOT_RECORD BootRecord = ( PVOLUME_BOOT_RECORD )0x7C00;
    PLOADER_BOOT_FILE   KernelFile;
    ULONG32             LoadAddress;
    ULONG64             EntryPoint;
    ULONG64             LoaderBlock;

    __outbyte( 0xA1, 0xFF );
    __outbyte( 0x21, 0xFF );

    _lgdt( &GlobalDescriptor );

    BdInitMap( );
    BdInitFile( );

    BdFindBootFile( KERNEL_FILE_NAME, &KernelFile );

    LoadAddress = ROUND_TO_PAGES(
        Loader.FileList[ Loader.FileCount - 1 ].BaseAddress +
        ( ULONG64 )Loader.FileList[ Loader.FileCount - 1 ].Length );

    LdrLoadSystemModule( BootRecord,
                         KernelFile,
                         LoadAddress );

    EntryPoint = BdAddress64( LdrEntryPoint( ( ULONG32 )Loader.MapList[ 0 ].BaseAddress ) );
    LoaderBlock = BdAddress64( ( ULONG32 )&Loader );

    BdDisplayLoader( );
    BdTranslateLoader( );

    BdInitGraphics( );
    x86Boot64( EntryPoint, LoaderBlock );
}
