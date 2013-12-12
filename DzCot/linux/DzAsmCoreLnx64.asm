#********************************************************************
#   @file       DzAsmCoreLnx64.asm
#   @brief      funciton used for switch cot in 64 bit linux
#   @author     Foreverflying <foreverflying@live.cn>
#   @date       2010/02/11
#********************************************************************

#PUBLIC CallDzCotEntry PROC     ; CallDzCotEntry
#PUBLIC DzSwitch PROC           ; DzSwitch

.extern DzCotEntry

.text

.globl CallDzCotEntry
.globl DzSwitch

# void __stdcall CallDzCotEntry( void )
CallDzCotEntry:
    leaq    16(%rsp), %rdx
    leaq    8(%rsp), %rsi
    movq    (%rsp), %rdi
    call    DzCotEntry

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
