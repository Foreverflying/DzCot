@********************************************************************
@   @file       DzAsmCoreArch.asm
@   @brief      funciton used for switch cot in linux arm32
@   @author     Foreverflying <foreverflying@live.cn>
@   @date       2016/04/06
@********************************************************************

@PUBLIC CallDzCotEntry PROC
@PUBLIC DzSwitch PROC

.section .text
.align 4
.arm

.globl CallDzCotEntry
.globl DzSwitch

@ void CallDzCotEntry( void )
CallDzCotEntry:
    mov     r0, r11             @ r11 is pointer of host
    add     r1, sp
    mov     r2, sp, #4
    bx      r10                 @ r10 is pointer of function DzCotEntry

@ void DzSwitch( DzHost* host, DzCot* dzCot );
@ host$ = r0
@ dzCot$ = r1
DzSwitch:
    push    {r4-r11, lr}

    ldr     r4, [r0]            @ r4 = host->currCot
    str     sp, [r4, #4]        @ host->currCot->sp = sp
    str     r1, [r0]            @ host->currCot = dzCot
    ldr     sp, [r1, #4]        @ sp = dzCot.sp
    
    pop     {r4-r11, pc}
