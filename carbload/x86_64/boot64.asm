
BITS        32

%DEFINE     ptr

%DEFINE     GDT_SEGMENT_R0_CODE64   28h
%DEFINE     GDT_SEGMENT_R0_DATA64   30h

%DEFINE     IA32_MSR_EFER           0C0000080h

%DEFINE     CR4_PAE                 (1 << 5)
%DEFINE     CR4_OSFXSR              (1 << 9)
%DEFINE     CR4_OSXMMEXCPT          (1 << 10)

%DEFINE     EFER_LME                (1 << 8)

%DEFINE     CR0_PG                  (1 << 31)
%DEFINE     CR0_MP                  (1 << 1)
%DEFINE     CR0_EM                  (1 << 2)
%DEFINE     CR0_NW                  (1 << 29)
%DEFINE     CR0_CD                  (1 << 30)

SECTION     .text

GLOBAL      _x86Boot64@16

_x86Boot64@16:
   
%DEFINE EntryAddress                4
%DEFINE LoaderBlock                 12

    mov     eax, cr4
    or      eax, CR4_PAE
    or      eax, CR4_OSFXSR
    or      eax, CR4_OSXMMEXCPT
    mov     cr4, eax

    mov     ecx, IA32_MSR_EFER
    rdmsr 
    or      eax, EFER_LME
    wrmsr

    mov     eax, cr0
    or      eax, CR0_PG
    mov     cr0, eax

    mov     ax, GDT_SEGMENT_R0_DATA64
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    jmp     GDT_SEGMENT_R0_CODE64:.x86_64_Entry

.x86_64_Entry:

BITS        64

    mov     rax, cr0
    or      rax, CR0_MP
    and     rax, ~CR0_EM
    and     rax, ~CR0_NW
    and     rax, ~CR0_CD
    mov     cr0, rax

    mov     rcx, qword ptr [rsp+LoaderBlock]
    mov     rdx, qword ptr [rsp+EntryAddress]
    sub     rsp, 30h
    and     rsp, ~0Fh
    jmp     rdx

