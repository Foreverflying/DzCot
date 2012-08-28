#********************************************************************
#    created:    2010/02/11 22:07
#    file:       DzAsmCoreLnx32.asm
#    author:     Foreverflying
#    purpose:    funciton used for switch cot on linux 32
#********************************************************************


#PUBLIC CallDzCotEntry PROC     ; CallDzCotEntry
#PUBLIC DzSwitch PROC           ; DzSwitch

.extern DzCotEntry

.text

.globl CallDzCotEntry
.globl DzSwitch

# void __stdcall CallDzCotEntry( void )
CallDzCotEntry:
    leal    8(%esp), %edx
    leal    4(%esp), %ecx
    movl    (%esp), %eax
    pushl   %edx
    pushl   %ecx
    pushl   %eax
    call    DzCotEntry

# void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );
# host$ = ecx
# dzCot$ = edx
DzSwitch:
    pushl   %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    movl    (%ecx), %esi    #esi = host->currCot
    movl    %esp, 4(%esi)   #host->currCot->sp = esp
    movl    %edx, (%ecx)    #host->currCot = dzCot
    movl    4(%edx), %esp   #esp = dzCot.sp

    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp

    ret