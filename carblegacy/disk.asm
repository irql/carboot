
BITS    16

%DEFINE BdDiskId BootDisk

BdDap:
    db 10h      ;DapLength
    db 0        ;Reserved
    dw 0        ;ReadCount
    dd 0        ;MemoryAddress
    dq 0        ;BlockAddress

BdReadSectors:
    pushad

    mov     dword ptr [BdDap+08h], eax
    mov     dword ptr [BdDap+0ch], edx
    mov     word  ptr [BdDap+02h], cx
    mov     byte  ptr [BdDap+00h], 10h
    mov     dword ptr [BdDap+04h], ebx

    mov     ah, 42h
    mov     dl, byte ptr [BdDiskId]
    mov     si, BdDap
    int     13h

    popad
    ret