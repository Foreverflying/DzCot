//********************************************************************
//   @file       DzAsmCoreArch.asm
//   @brief      funciton used for switch cot in linux arm64
//   @author     Foreverflying <foreverflying//live.cn>
//   @date       2022/01/09
//********************************************************************

//PUBLIC CallDzCotEntry PROC
//PUBLIC DzSwitch PROC

.section .text
.align 8
.arch armv8-a+nofp

.globl CallDzCotEntry
.globl DzSwitch

// void CallDzCotEntry(void)
CallDzCotEntry:
    mov     x0, x29             // x29 is pointer of host
    mov     x1, sp
    add     x2, sp, #8
    br      x28                 // x28 is pointer of function DzCotEntry

// void DzSwitch(DzHost* host, DzCot* dzCot);
// host$ = x0
// dzCot$ = x1
DzSwitch:
    stp     x29, x30, [sp, #-16]
    stp     x27, x28, [sp, #-32]
    stp     x25, x26, [sp, #-48]
    stp     x23, x24, [sp, #-64]
    stp     x21, x22, [sp, #-80]
    stp     x19, x20, [sp, #-96]

    ldr     x19, [x0]           // x19 = host->currCot
    add     x20, sp, #-96       // currentSp = originalSp + 96
    str     x20, [x19, #8]      // host->currCot->sp = currentSp
    str     x1, [x0]            // host->currCot = dzCot
    ldr     x21, [x1, #8]       // currentSp = dzCot.sp
    add     sp, x21, #96        // original sp = currentSp - 96

    ldp     x19, x20, [sp, #-96]
    ldp     x21, x22, [sp, #-80]
    ldp     x23, x24, [sp, #-64]
    ldp     x25, x26, [sp, #-48]
    ldp     x27, x28, [sp, #-32]
    ldp     x29, x30, [sp, #-16]

    ret
