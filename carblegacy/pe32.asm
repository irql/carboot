
BITS    32

%DEFINE sizeof_FileHeader           14h
%DEFINE sizeof_SectionHeader        28h
%DEFINE sizeof_DataDirectory        08h
%DEFINE sizeof_BaseReloc            08h

%DEFINE dos_e_lfanew                3ch

%DEFINE nt_FileHeader				4h
%DEFINE nt_OptionalHeader			nt_FileHeader+sizeof_FileHeader

%DEFINE fh_SizeOfOptionalHeader		10h
%DEFINE fh_NumberOfSections			2h

%DEFINE oh_AddressOfEntryPoint		10h
%DEFINE oh_ImageBase				1ch
%DEFINE oh_SizeOfHeaders			3ch
%DEFINE oh_DataDirectory			60h

%DEFINE sh_VirtualSize				8h
%DEFINE sh_VirtualAddress			0ch
%DEFINE sh_RawSize					10h
%DEFINE sh_RawAddress				14h
%DEFINE sh_Characteristics			24h

%DEFINE br_VirtualAddress			0h
%DEFINE br_SizeOfBlock				4h

BdPe32LoadImage:
    
    ; STDCALL ULONG32 BdPe32( ULONG32 FileBase );

%DEFINE FileBase                +64
%DEFINE HeaderDos               +4
%DEFINE HeaderNt                +8
%DEFINE ImageBase               +12
%DEFINE HeaderSections          +16
%DEFINE CurrentSection          +20
%DEFINE CurrentSectionAddress   +24
%DEFINE PreviousSectionImage    +28
%DEFINE PreviousSectionFile     +32
%DEFINE BaseReloc               +36
%DEFINE BaseDelta               +40
%DEFINE CurrentReloc            +44

    push    ebp
    push    esi
    push    edi
    sub     esp, 48

    mov     esi, dword ptr FileBase[esp]
    mov     edx, esi
    add     edx, dword ptr [esi+dos_e_lfanew]

    movzx   eax, word ptr [edx+nt_FileHeader+fh_SizeOfOptionalHeader]
    lea     eax, [eax+edx+nt_OptionalHeader]
    mov     ecx, dword ptr [edx+nt_OptionalHeader+oh_SizeOfHeaders]
    mov     edi, 40000h

    ; Esi = Dos
    ; Edx = Nt
    ; Ecx = SizeOfHeaders
    ; Eax = SectionHeaders
    ; Edi = ImageBase

    mov     dword ptr HeaderDos[esp], esi
    mov     dword ptr ImageBase[esp], edi
    mov     dword ptr HeaderNt[esp], edx
    mov     dword ptr HeaderSections[esp], eax

    cld
    rep     movsb

    mov     esi, dword ptr HeaderDos[esp]
    mov     edi, dword ptr ImageBase[esp]

    movzx   ebp, word ptr [edx+nt_FileHeader+fh_NumberOfSections]
    xor     ecx, ecx
.BpPe32LoadSection:
    mov     dword ptr CurrentSection[esp], ecx
    imul    ecx, sizeof_SectionHeader
    
    mov     eax, dword ptr HeaderSections[esp]
    add     eax, ecx
    mov     dword ptr CurrentSectionAddress[esp], eax
    
    mov     esi, dword ptr HeaderDos[esp]
    mov     edi, dword ptr ImageBase[esp]
    add     esi, dword ptr [eax+sh_RawAddress]
    add     edi, dword ptr [eax+sh_VirtualAddress]

    cld
    mov     ecx, dword ptr [eax+sh_VirtualAddress]
    mov     dword ptr PreviousSectionFile[esp], esi
    mov     dword ptr PreviousSectionImage[esp], edi
    xor     eax, eax
    rep     stosb
    mov     eax, dword ptr HeaderSections[esp]
    mov     ecx, dword ptr [eax+sh_VirtualSize]
    mov     esi, dword ptr PreviousSectionFile[esp]
    mov     edi, dword ptr PreviousSectionImage[esp]
    rep     movsb

    mov     ecx, dword ptr CurrentSection[esp]
    inc     ecx
    cmp     ecx, ebp
    jne     .BpPe32LoadSection

    mov     esi, dword ptr HeaderDos[esp]
    mov     edi, dword ptr ImageBase[esp]
    mov     edx, dword ptr HeaderNt[esp]
    mov     ecx, edi
    sub     ecx, dword ptr [edx+nt_OptionalHeader+oh_ImageBase]
    
    mov     eax, dword ptr [edx+nt_OptionalHeader+oh_DataDirectory+5*sizeof_DataDirectory]
    test    eax, eax
    jz      .BpPe32Success
    add     eax, edi

    ; Ecx = BaseDelta
    ; Eax = BaseReloc

    mov     dword ptr BaseDelta[esp], ecx
    mov     dword ptr BaseReloc[esp], eax

.BpPe32Reloc:
    cmp     dword ptr [eax+br_VirtualAddress], 0
    je      .BpPe32Success
    
    mov     dword ptr BaseReloc[esp], eax
    mov     ecx, dword ptr [eax+br_SizeOfBlock]
    cmp     ecx, sizeof_BaseReloc
    jle     .BpPe32AdvanceReloc

    sub     ecx, sizeof_BaseReloc
    add     eax, sizeof_BaseReloc
.BpPe32RelocThunk:
    cmp     word ptr [eax], 0
    je      .BpPe32ThunkClean
    
    mov     dword ptr CurrentReloc[esp], ecx
    movzx   ecx, word ptr [eax]
    and     ecx, 0fffh
    mov     ebp, dword ptr BaseReloc[esp]
    add     ecx, dword ptr [ebp+br_VirtualAddress]
    add     ecx, dword ptr ImageBase[esp]
    mov     ebp, dword ptr BaseDelta[esp]
    add     dword ptr [ecx], ebp
    mov     ecx, dword ptr CurrentReloc[esp]

.BpPe32ThunkClean:
    add     eax, 2
    sub     ecx, 2
    jnz     .BpPe32RelocThunk
.BpPe32AdvanceReloc:
    mov     eax, dword ptr BaseReloc[esp]
    add     eax, dword ptr [eax+br_SizeOfBlock] 
    jmp     .BpPe32Reloc

.BpPe32Failure:
    mov     eax, 0D3ADB00Bh
    cli
    hlt
.BpPe32Success:
    mov     eax, dword ptr HeaderNt[esp]
    mov     eax, dword ptr [eax+nt_OptionalHeader+oh_AddressOfEntryPoint]
    add     eax, dword ptr ImageBase[esp]

    add     esp, 48
    pop     edi
    pop     esi
    pop     ebp
    ret     4
