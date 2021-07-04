
#include "loader.h"

NTSTATUS
LdrGetExportAddressByName(
    _In_  PVOID    Base,
    _In_  PCHAR    Name,
    _Out_ ULONG64* Address
)
{
    PIMAGE_DOS_HEADER       HeaderDos;
    PIMAGE_NT_HEADERS       HeaderNt;
    PIMAGE_EXPORT_DIRECTORY Export;
    PULONG32                FunctionTable;
    PULONG32                NameTable;
    PUSHORT                 OrdinalTable;
    ULONG32                 CurrentOrdinal;
    PCHAR                   CurrentName;

    HeaderDos = ( PIMAGE_DOS_HEADER )Base;

    if ( HeaderDos->e_magic != IMAGE_DOS_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    HeaderNt = ( PIMAGE_NT_HEADERS )( ( PUCHAR )Base + HeaderDos->e_lfanew );

    if ( HeaderNt->Signature != IMAGE_NT_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    if ( HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].Size == 0 ) {

        return STATUS_INVALID_IMAGE;
    }

    Export = ( PIMAGE_EXPORT_DIRECTORY )( ( PUCHAR )Base + HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );
    FunctionTable = ( PULONG32 )( ( PUCHAR )Base + Export->AddressOfFunctions );
    NameTable = ( PULONG32 )( ( PUCHAR )Base + Export->AddressOfNames );
    OrdinalTable = ( PUSHORT )( ( PUCHAR )Base + Export->AddressOfNameOrdinals );

    for ( CurrentOrdinal = 0; CurrentOrdinal < Export->NumberOfNames; CurrentOrdinal++ ) {
        CurrentName = ( ( PCHAR )Base + NameTable[ CurrentOrdinal ] );

        if ( RtlCompareAnsiString( Name, CurrentName, 0 ) == 0 ) {
            *Address = ( ULONG64 )( ( PUCHAR )Base + FunctionTable[ OrdinalTable[ CurrentOrdinal ] ] );
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
LdrGetExportAddressByOrdinal(
    _In_  PVOID    Base,
    _In_  USHORT   Ordinal,
    _Out_ ULONG64* Address
)
{
    PIMAGE_DOS_HEADER       HeaderDos;
    PIMAGE_NT_HEADERS       HeaderNt;
    PIMAGE_EXPORT_DIRECTORY Export;
    PULONG32                FunctionTable;
    PUSHORT                 OrdinalTable;
    ULONG32                 CurrentOrdinal;
    USHORT                  OrdinalValue;

    HeaderDos = ( PIMAGE_DOS_HEADER )Base;

    if ( HeaderDos->e_magic != IMAGE_DOS_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    HeaderNt = ( PIMAGE_NT_HEADERS )( ( PUCHAR )Base + HeaderDos->e_lfanew );

    if ( HeaderNt->Signature != IMAGE_NT_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    if ( HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].Size == 0 ) {

        return STATUS_NOT_FOUND;
    }

    Export = ( PIMAGE_EXPORT_DIRECTORY )( ( PUCHAR )Base + HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );
    FunctionTable = ( PULONG32 )( ( PUCHAR )Base + Export->AddressOfFunctions );
    OrdinalTable = ( PUSHORT )( ( PUCHAR )Base + Export->AddressOfNameOrdinals );

    for ( CurrentOrdinal = 0; CurrentOrdinal < Export->NumberOfFunctions; CurrentOrdinal++ ) {
        OrdinalValue = *( PUSHORT )( ( PUCHAR )Base + OrdinalTable[ CurrentOrdinal ] );

        if ( OrdinalValue == Ordinal ) {
            *Address = ( ULONG64 )( ( PUCHAR )Base + FunctionTable[ OrdinalTable[ CurrentOrdinal ] ] );
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
LdrResolveBaseReloc(
    _In_ ULONG32 BaseAddress
)
{
    PIMAGE_DOS_HEADER       HeaderDos;
    PIMAGE_NT_HEADERS       HeaderNt;
    PIMAGE_BASE_RELOCATION  BaseReloc;
    LONG64                  BaseDelta;
    ULONG64                 RelocCount;
    PUSHORT                 RelocList;
    ULONG64                 CurrentReloc;
    PLONG64                 Address;

    HeaderDos = ( PIMAGE_DOS_HEADER )( BaseAddress );

    if ( HeaderDos->e_magic != IMAGE_DOS_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    HeaderNt = ( PIMAGE_NT_HEADERS )( BaseAddress + HeaderDos->e_lfanew );

    if ( HeaderNt->Signature != IMAGE_NT_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    if ( HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].Size == 0 ||
        ( HeaderNt->OptionalHeader.DllCharacteristics & IMAGE_FILE_RELOCS_STRIPPED ) != 0 ) {

        return STATUS_INVALID_IMAGE;
    }

    BaseReloc = ( PIMAGE_BASE_RELOCATION )( BaseAddress + HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress );
    BaseDelta = BdAddress64( BaseAddress ) - HeaderNt->OptionalHeader.ImageBase;

    if ( BaseDelta == 0 ) {

        return STATUS_SUCCESS;
    }

    while ( BaseReloc->VirtualAddress ) {

        if ( BaseReloc->SizeOfBlock > sizeof( IMAGE_BASE_RELOCATION ) ) {
            RelocCount = ( BaseReloc->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( USHORT );
            RelocList = ( PUSHORT )( ( PUCHAR )BaseReloc + sizeof( IMAGE_BASE_RELOCATION ) );

            for ( CurrentReloc = 0; CurrentReloc < RelocCount; CurrentReloc++ ) {

                if ( RelocList[ CurrentReloc ] != 0 ) {
                    Address = ( PLONG64 )( BaseAddress + BaseReloc->VirtualAddress + ( RelocList[ CurrentReloc ] & 0xfff ) );
                    *Address += BaseDelta;
                }
            }
        }

        BaseReloc = ( PIMAGE_BASE_RELOCATION )( ( PUCHAR )BaseReloc + BaseReloc->SizeOfBlock );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LdrResolveImportTable(
    _In_ PVOID                    Importer,
    _In_ PVOID                    Importee,
    _In_ PIMAGE_IMPORT_DESCRIPTOR Import
)
{
    NTSTATUS              ntStatus;
    PIMAGE_THUNK_DATA     OriginalFirstThunk;
    PIMAGE_THUNK_DATA     FirstThunk;
    PIMAGE_IMPORT_BY_NAME Name;

    OriginalFirstThunk = ( PIMAGE_THUNK_DATA )( ( PUCHAR )Importer + Import->OriginalFirstThunk );
    FirstThunk = ( PIMAGE_THUNK_DATA )( ( PUCHAR )Importer + Import->FirstThunk );

    while ( OriginalFirstThunk->u1.AddressOfData ) {

        if ( OriginalFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG ) {
            ntStatus = LdrGetExportAddressByOrdinal(
                Importee,
                ( USHORT )( OriginalFirstThunk->u1.Ordinal & 0xFFFF ),
                &FirstThunk->u1.Function );

            if ( !NT_SUCCESS( ntStatus ) ) {

                return ntStatus;
            }

            FirstThunk->u1.Function = BdAddress64( FirstThunk->u1.Function );
        }
        else {
            Name = ( PIMAGE_IMPORT_BY_NAME )( ( PUCHAR )Importer + OriginalFirstThunk->u1.AddressOfData );
            ntStatus = LdrGetExportAddressByName(
                Importee,
                ( PCHAR )Name->Name,
                &FirstThunk->u1.Function );

            if ( !NT_SUCCESS( ntStatus ) ) {

                return ntStatus;
            }

            FirstThunk->u1.Function = BdAddress64( FirstThunk->u1.Function );
        }

        OriginalFirstThunk++;
        FirstThunk++;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LdrLoadSystemModule(
    _In_  PVOLUME_BOOT_RECORD BootRecord,
    _In_  PLOADER_BOOT_FILE   BootFile,
    _In_  ULONG32             BaseAddress
)
{
    NTSTATUS                 ntStatus;
    PIMAGE_DOS_HEADER        HeaderDos;
    PIMAGE_NT_HEADERS        HeaderNt;
    PIMAGE_SECTION_HEADER    HeaderSection;
    PIMAGE_SECTION_HEADER    LastSection;
    PIMAGE_IMPORT_DESCRIPTOR Import;
    ULONG32                  CurrentSection;
    ULONG32                  CurrentMap;
    PLOADER_SYSTEM_MAP       ImportMap;
    PLOADER_BOOT_FILE        ImportFile;

    HeaderDos = ( PIMAGE_DOS_HEADER )( ( ULONG32 )BootFile->BaseAddress );

    if ( HeaderDos->e_magic != IMAGE_DOS_SIGNATURE ) {

        return STATUS_INVALID_IMAGE;
    }

    HeaderNt = ( PIMAGE_NT_HEADERS )( ( ULONG32 )BootFile->BaseAddress + HeaderDos->e_lfanew );

    if ( HeaderNt->Signature != IMAGE_NT_SIGNATURE ||
         HeaderNt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ) {

        return STATUS_INVALID_IMAGE;
    }

    HeaderSection = IMAGE_FIRST_SECTION( HeaderNt );
    LastSection = &HeaderSection[ HeaderNt->FileHeader.NumberOfSections - 1 ];

    CurrentMap = Loader.MapCount;
    Loader.MapList[ CurrentMap ].BaseAddress = BaseAddress;
    Loader.MapList[ CurrentMap ].Length = LastSection->VirtualAddress +
        ROUND_TO_PAGES( LastSection->Misc.VirtualSize );
    Loader.MapList[ CurrentMap ].BootFile = BootFile;

    Loader.MapCount++;

    RtlCopyMemory( ( PVOID )BaseAddress,
                   HeaderDos,
                   HeaderNt->OptionalHeader.SizeOfHeaders );

    for ( CurrentSection = 0;
          CurrentSection < HeaderNt->FileHeader.NumberOfSections;
          CurrentSection++ ) {

        RtlCopyMemory(
            ( PUCHAR )BaseAddress + HeaderSection[ CurrentSection ].VirtualAddress,
            ( PUCHAR )BootFile->BaseAddress + HeaderSection[ CurrentSection ].PointerToRawData,
            HeaderSection[ CurrentSection ].SizeOfRawData );
    }

    if (
        ( HeaderNt->OptionalHeader.DllCharacteristics & IMAGE_FILE_RELOCS_STRIPPED ) == 0 &&
        ( HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress ) != 0 &&
        ( BdAddress64( BaseAddress ) != HeaderNt->OptionalHeader.ImageBase ) ) {

        LdrResolveBaseReloc( BaseAddress );
    }

    if ( HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress != 0 ) {

        Import = ( PIMAGE_IMPORT_DESCRIPTOR )( BaseAddress +
                                               HeaderNt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress );

        while ( Import->Characteristics ) {

            BdFindSystemFile( ( PSTR )( BaseAddress + Import->Name ),
                              &ImportMap );

            if ( ImportMap == NULL ) {

                BdFindBootFile( ( PSTR )( BaseAddress + Import->Name ),
                                &ImportFile );

                if ( ImportFile == NULL ) {

                    BpFatalException( "FATAL: failed to find imported module \"%s\".", ( PSTR )( BaseAddress + Import->Name ) );
                }

                ntStatus = LdrLoadSystemModule( BootRecord,
                                                ImportFile,
                                                Loader.MapList[ Loader.MapCount - 1 ].BaseAddress +
                                                Loader.MapList[ Loader.MapCount - 1 ].Length );

                if ( !NT_SUCCESS( ntStatus ) ) {

                    BpFatalException( "FATAL: failed to load \"%s\".", ImportFile->FileName );
                }

                BdFindSystemFile( ( PSTR )( BaseAddress + Import->Name ),
                                  &ImportMap );
            }

            ntStatus = LdrResolveImportTable(
                ( PVOID )BaseAddress,
                ( PVOID )ImportMap->BaseAddress,
                Import );

            if ( !NT_SUCCESS( ntStatus ) ) {

                BpFatalException( "FATAL: failed to resolve imports for module \"%s\".", ( PSTR )( BaseAddress + Import->Name ) );
            }

            Import++;
        }
    }

    return STATUS_SUCCESS;
}

ULONG32
LdrEntryPoint(
    _In_ ULONG32 BaseAddress
)
{
    PIMAGE_NT_HEADERS HeaderNt;

    HeaderNt = ( PIMAGE_NT_HEADERS )( BaseAddress + ( ( PIMAGE_DOS_HEADER )( BaseAddress ) )->e_lfanew );

    return BaseAddress + HeaderNt->OptionalHeader.AddressOfEntryPoint;
}
