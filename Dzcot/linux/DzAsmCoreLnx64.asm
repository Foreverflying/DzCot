#********************************************************************
#    created:    2010/02/11 22:07
#    file:       DzAsmCoreLnx32.asm
#    author:     Foreverflying
#    purpose:    funciton used for switch cot on linux 32
#********************************************************************


#PUBLIC @DzSwitch@8 PROC    ; DzSwitch

        .text

.globl DzSwitchFast
        .type  DzSwitchFast, @function
# void __fastcall DzSwitch( DzHost* host, DzThread* dzThread );
# host$ = rdi
# dzThread$ = rsi
DzSwitchFast:
        pushq   %rbp
        pushq   %rbx
        pushq   %r12
        pushq   %r13
        pushq   %r14
        pushq   %r15

        movl    (%rdi), %rax    #rax = host->currThread
        movl    %rsp, 8(%rax)   #host->currThread->sp = rsp
        movl    %rsi, (%rax)    #host->currThread = dzThread
        movl    8(%rsi), %rsp   #rsp = dzThread.sp

        popq    %r15
        popq    %r14
        popq    %r13
        popq    %r12
        popq    %rbx
        popq    %rbp

        ret
