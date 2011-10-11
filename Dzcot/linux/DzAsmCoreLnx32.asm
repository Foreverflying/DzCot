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
    leal    4(%esp), %eax
    movl    %esp, %edx
    pushl   %eax
    pushl   %edx
    call    DzcotEntry

# void __fastcall DzSwitch( DzHost* host, DzThread* dzThread );
# host$ = ecx
# dzThread$ = edx
DzSwitch:
    pushl   %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    movl    (%ecx), %esi    #esi = host->currThread
    movl    %esp, 4(%esi)   #host->currThread->sp = esp
    movl    %edx, (%ecx)    #host->currThread = dzThread
    movl    4(%edx), %esp   #esp = dzThread.sp

    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp

    ret
