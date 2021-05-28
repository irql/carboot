
BITS    16
ORG     7c00h

jmp     InitLoad
nop

Bpb2_00:
FileSystemIdentifier        db "MSDOS5.0"
BytesPerSector              dw 512
SectorsPerCluster           db 0
ReservedSectors             dw 0
FatCount                    db 2
RootDirectoryEntriesCount   dw 0
TotalSectors16              dw 0
MediaDescriptor             db 0f8h
SectorsPerFat16             dw 0

Bpb3_31:
SectorsPerTrack             dw 0
NumberOfHeads               dw 0
HiddenSectors               dd 0
TotalSectors32              dd 0

Bpb7_01:
SectorsPerFat32             dd 0
MirroringFlags              dw 0
FatVersion                  dw 0
RootDirectoryCluster        dd 2
FileSystemInfoSector        dw 1
BackupSectors               dw 6
TIMES 12                    db 0
BootDisk                    db 80h
NtFlags                     db 0
ExtendedBootSignature       db 29h
VolumeSerialNumber          dd 0
VolumeLabel                 db "NO NAME    "
SystemIdentifierString      db "FAT32   "

TIMES 90 - ($ - $$)         db ' '

%DEFINE ptr

ALIGN   4
InitLoad:

    cli
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, 7c00h
    mov     byte [BootDisk], dl
    jmp     0:Load

%INCLUDE "disk.asm"

BdSystemDirectory       db "SYSTEM     "
BdLoaderDirectoryFile   db "CARBLOADSYS"

BdLoaderBuffer          dd 2000_0000h

ALIGN   4

Load:

    in      al, 92h
    or      al, 02h
    out     92h, al

    movzx   eax, word ptr [BackupSectors]
    add     eax, dword ptr [HiddenSectors]
    add     eax, 3
    xor     edx, edx
    mov     ecx, 3
    mov     ebx, 7e00h
    call    BdReadSectors

    mov     edx, 2
    mov     ebx, dword ptr [BdLoaderBuffer]
    call    BdFat32ReadCluster

    mov     ebx, dword ptr [BdLoaderBuffer]
    mov     eax, ebx
    mov     ecx, BdSystemDirectory
    call    BdFat32ReadDirectoryFile

    mov     ebx, dword ptr [BdLoaderBuffer]
    mov     eax, ebx
    mov     ecx, BdLoaderDirectoryFile
    call    BdFat32ReadDirectoryFile

    lgdt    [GlobalDescriptorTable]
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    mov     ax, 16
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    jmp     8:Load2

BITS    32

Load2:

    ; 32 bit code is all stdcall

    mov     ecx, dword ptr [BdLoaderBuffer]
    shr     ecx, 12
    add     cx, word ptr [BdLoaderBuffer]
    push    ecx
    call    BdPe32LoadImage
    jmp     eax

SegmentTable:
    dq      0
    
    dw      0ffffh
    dw      0
    db      0
    db      10011010b ;db      10011010b
    db      11001111b
    db      0

    dw      0ffffh
    dw      0
    db      0
    db      10010010b
    db      11001111b
    db      0

GlobalDescriptorTable:
    dw      17h
    dd      SegmentTable

%ASSIGN     BYTES_LEFT 510 - ($ - $$)
%WARNING    SECTOR 0 HAS BYTES_LEFT BYTES LEFT.
TIMES       BYTES_LEFT db   0
dw                          0xAA55

%INCLUDE "fat32.asm"
%INCLUDE "pe32.asm"

%ASSIGN     BYTES_LEFT 2046 - ($ - $$)
%WARNING    SECTOR 1 HAS BYTES_LEFT BYTES LEFT.
TIMES       BYTES_LEFT db   0
dw                          0xAA55