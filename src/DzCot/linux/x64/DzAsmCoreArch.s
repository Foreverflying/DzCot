#********************************************************************
#   @file       DzAsmCoreArch.asm
#   @brief      funciton used for switch cot in linux x64
#   @author     Foreverflying <foreverflying@live.cn>
#   @date       2010/02/11
#********************************************************************

#PUBLIC CallDzCotEntry PROC
#PUBLIC DzSwitch PROC

.text

.globl CallDzCotEntry
.globl DzSwitch

# void __stdcall CallDzCotEntry( void )
CallDzCotEntry:
    leaq    8(%rsp), %rdx
    leaq    (%rsp), %rsi
    movq    %r12, %rdi          # r12 is pointer of host
    call    *%r13               # r13 is pointer of function DzCotEntry

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

    movq    (%rdi), %rax        # rax = host->currCot
    movq    %rsp, 8(%rax)       # host->currCot->sp = rsp
    movq    %rsi, (%rdi)        # host->currCot = dzCot
    movq    8(%rsi), %rsp       # rsp = dzCot.sp

    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp

    ret
