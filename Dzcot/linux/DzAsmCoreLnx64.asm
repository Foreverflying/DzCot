#********************************************************************
#    created:    2010/02/11 22:07
#    file:       DzAsmCoreLnx32.asm
#    author:     Foreverflying
#    purpose:    funciton used for switch cot on linux 32
#********************************************************************


#PUBLIC CallDzcotEntry PROC     ; CallDzcotEntry
#PUBLIC DzSwitch PROC           ; DzSwitch

.extern DzcotEntry

.text

.globl CallDzcotEntry
.globl DzSwitch

# void __stdcall CallDzcotEntry( void )
CallDzcotEntry:
    leaq    16(%rsp), %rdx
    leaq    8(%rsp), %rsi
    movq    (%rsp), %rdi
    call    DzcotEntry

# void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );
# host$ = rdi
# dzCot$ = rsi
DzSwitch:
    pushq   %rbp
    pushq   %rbx
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    movq    (%rdi), %rax    #rax = host->currCot
    movq    %rsp, 8(%rax)   #host->currCot->sp = rsp
    movq    %rsi, (%rdi)    #host->currCot = dzCot
    movq    8(%rsi), %rsp   #rsp = dzCot.sp

    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp

    ret
