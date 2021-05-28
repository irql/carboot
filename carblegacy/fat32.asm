
BITS    16

%DEFINE CLUSTER_LOWER   26
%DEFINE CLUSTER_UPPER   20

BpFat32FirstClusterBlockAddress:

    ;
    ;   evaluates the formula below, taken
    ;   from fatgen103.
    ;
    ;   Bpb->Dos2_00Bpb.ReservedSectors + 
    ;   Bpb->Dos3_31Bpb.HiddenSectors + 
    ;   Bpb->Dos2_00Bpb.FatCount * 
    ;   Bpb->Dos7_01Bpb.SectorsPerFat
    ;

    push    ebx
    push    edx

    movzx   ax, byte ptr [FatCount]
    mul     word ptr [SectorsPerFat32]

    mov     bx, dx
    shl     ebx, 16
    mov     bx, ax

    add     bx, word ptr [ReservedSectors]
    add     ebx, dword ptr [HiddenSectors]

    mov     eax, ebx
    pop     edx
    pop     ebx
    ret

BdFat32ReadCluster:

    ;
    ;   EDX = ClusterNumber
    ;   EBX = Segment:Offset
    ;   Read's the cluster specified by EDX
    ;   into the buffer specified by EBX.
    ;

    pushad
    push    ebx

    call    BpFat32FirstClusterBlockAddress
    mov     ebx, eax

    ;
    ;   the fat32, 2 cluster sub
    ;

    sub     edx, 2

    ;
    ;   calculate FirstCluster + SectorsPerCluster * ClusterNumber
    ;

    movzx   eax, byte ptr [SectorsPerCluster]
    mul     edx
    add     eax, ebx

    pop     ebx
    movzx   cx, byte ptr [SectorsPerCluster]
    call    BdReadSectors
    
    popad
    ret

BdFat32ReadClusterChain:

    ;
    ;   EDX = ClusterNumber
    ;   EBX = Segment:Offset
    ;
    ;   returns:
    ;   EBX = Segment:Offset of next free memory
    ;         region.
    ;
    ;   Reads a chain of clusters.
    ;

    pushad

.BpFat32ChainLoop:
    call    BdFat32ReadCluster
    push    ebx

    ;
    ;   calculates the EDX = FatSectorIndex and
    ;   EAX = FatSectorOffset 
    ;

    mov     eax, 4
    mul     edx
    movzx   ecx, word ptr [BytesPerSector]
    div     ecx

    ;
    ;   this code reads the FatSectorIndex into memory
    ;

    push    edx
    xor     edx, edx
    
    add     ax, word ptr [ReservedSectors]
    add     eax, dword ptr [HiddenSectors]
    mov     ebx, 4000h
    mov     cx, 1
    call    BdReadSectors

    ;
    ;   load the new cluster number in edx
    ;   and use (SectorsPerCluster * BytesPerSector) / 0x10 to 
    ;   calculate a new segment:offset pointer. (we're currently
    ;   ignoring the offset field of segment:offset pointers)
    ;

    pop     edx
    mov     edx, dword ptr [edx+4000h]
    push    edx

    movzx   eax, byte ptr [SectorsPerCluster]
    mov     cx, word ptr [BytesPerSector]
    mul     cx
    mov     bx, 16
    div     bx

    mov     ecx, eax
    shl     ecx, 16

    pop     edx
    pop     ebx
    add     ebx, ecx
    cmp     edx, 0x0fffffff
    jne     .BpFat32ChainLoop

    mov     dword [esp+16], ebx
    popad
    ret

BdFat32ReadDirectoryFile:

    ;
    ;   EAX = Directory
    ;   EBX = Segment:Offset
    ;    CX = FileNameOffset
    ;   reads a fat32 file from a directory
    ;   uses 4000h as a temporary buffer.
    ;

    pushad
    mov     ax, es
    push    eax
    push    ebx
    mov     bx, cx

    mov     di, ax
    shr     eax, 16
    mov     es, ax

    ;
    ;   check each entry in the directory
    ;   comparing the 11 char name to the one
    ;   passed to this function, if the first byte
    ;   is 0, we can assume that is the end of the 
    ;   directory listing, and jump to a failure
    ;   handler.
    ;

.BpFat32DirectoryEntry:
    add     di, 32
    mov     si, bx
    cmp     byte ptr es:[di], 0
    je      .BpFat32Failure

    push    edi
    mov     cx, 11
    repe    cmpsb
    pop     edi
    jne     .BpFat32DirectoryEntry

    ;
    ;   the code escaped the directory loop,
    ;   meaning it must've found our entry.
    ;   EDX = ClusterNumber
    ;

    mov     dx, word ptr es:[di+CLUSTER_UPPER]
    shl     edx, 16
    mov     dx, word ptr es:[di+CLUSTER_LOWER]

    pop     ebx

    call    BdFat32ReadClusterChain
    jmp     .BpFat32Success

.BpFat32Failure:
    mov     eax, 0D3ADB00Bh
    cli
    hlt
.BpFat32Success:
    
    pop     eax
    mov     es, ax
    mov     dword ptr [esp+16], ebx
    popad
    ret