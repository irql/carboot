
BITS        32

%DEFINE     ptr

%DEFINE     GDT_SEGMENT_R0_CODE32   08h
%DEFINE     GDT_SEGMENT_R0_DATA32   10h
%DEFINE     GDT_SEGMENT_R0_CODE16   18h
%DEFINE     GDT_SEGMENT_R0_DATA16   20h

SECTION     .text

EXTERN      _BdExitPagedMode@0
EXTERN      _BdEnterPagedMode@0

GLOBAL      _x86BiosCall@8

_x86BiosCall@8:

%DEFINE     Vector    20
%DEFINE     BiosFrame 24

    push    edi
    push    esi
    push    ebp
    push    ebx

    call    _BdExitPagedMode@0
    mov     ecx, dword ptr [esp+BiosFrame]
    mov     edx, dword ptr [esp+Vector]
    mov     byte ptr [.x86_16_Intr+1], dl

    ;
    ; mov     eax, cr0
    ; and     eax, ~1
    ; mov     cr0, eax
    ; jmp     far ptr .x86_16_Enter
    ;
    mov     dword ptr [0x1000], 0x83C0200F
    mov     dword ptr [0x1004], 0x220FFEE0
    mov     word ptr [0x1008], 0xEAC0
    mov     edx, .x86_16_Enter
    shr     edx, 16
    shl     edx, 12
    mov     word ptr [0x100C], dx
    mov     edx, .x86_16_Enter
    and     edx, 0ffffh
    mov     word ptr [0x100A], dx

    mov     ax, GDT_SEGMENT_R0_DATA16
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    jmp     GDT_SEGMENT_R0_CODE16:0x1000

.x86_16_Enter:

BITS        16
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    mov     dword ptr [ecx+16], esp
    mov     esp, ecx
    pop     ax
    mov     fs, ax
    pop     ax
    mov     es, ax
    popad
    mov     esp, dword ptr [esp-20]
    push    ax
    mov     ax, fs
    mov     ds, ax
    xor     ax, ax
    mov     fs, ax
    pop     ax

.x86_16_Intr:
    int     0

.x86_16_Leave:

    mov     dword ptr [esp-4], ecx
    mov     cx, ds
    mov     fs, cx
    xor     cx, cx
    mov     ds, cx
    mov     ecx, dword ptr [esp+BiosFrame]
    mov     dword ptr [ecx+4], edi 
    mov     dword ptr [ecx+8], esi 
    mov     dword ptr [ecx+12], ebp 
    mov     dword ptr [ecx+20], ebx
    mov     dword ptr [ecx+24], edx
    mov     edx, dword ptr [esp-4]
    mov     dword ptr [ecx+28], edx
    mov     dword ptr [ecx+32], eax
    mov     ax, fs
    mov     word ptr [ecx], ax
    mov     ax, es
    mov     word ptr [ecx+2], ax

    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    mov     ax, GDT_SEGMENT_R0_DATA32
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    sub     esp, 8
    mov     dword ptr [esp+4], GDT_SEGMENT_R0_CODE32
    mov     dword ptr [esp], .x86_32_Done
    o32     retf
.x86_32_Done:

BITS        32
    call    _BdEnterPagedMode@0
    pop     ebx
    pop     ebp
    pop     esi
    pop     edi
    ret     8
