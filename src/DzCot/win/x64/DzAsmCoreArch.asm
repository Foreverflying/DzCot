;********************************************************************
;   @file       DzAsmCoreArch.asm
;   @brief      funciton used for switch cot in 64 bit windows
;   @author     Foreverflying <foreverflying@live.cn>
;   @date       2010/02/11
;********************************************************************

;PUBLIC CallDzCotEntry PROC     ; CallDzCotEntry
;PUBLIC DzSwitch PROC           ; DzSwitch

EXTRN   DzCotEntry : PROC

_TEXT   SEGMENT

; void CallDzCotEntry( void );
CallDzCotEntry PROC
    lea     r8, [rsp+8]
    lea     rdx, [rsp]
    mov     rcx, rsi            ; rsi is pointer of host
    sub     rsp, 32             ; make spill slot
    call    rdi                 ; rdi is pointer of function DzCotEntry
CallDzCotEntry ENDP

; void DzSwitch( DzHost* host, DzCot* dzCot );
; host$ = rcx
; dzCot$ = rdx
DzSwitch PROC
    push    rbp
    push    rbx
    push    rsi
    push    rdi
    push    r12
    push    r13
    push    r14
    push    r15

    mov     rax, gs:[30h]
    push    qword ptr [rax+8]
    push    qword ptr [rax+16]

    mov     rsi, [rcx]          ; rsi = host->currCot
    mov     [rsi+8], rsp        ; host->currCot->sp = rsp
    mov     [rcx], rdx          ; host->currCot = dzCot
    mov     rsp, [rdx+8]        ; rsp = dzCot.sp

    pop     qword ptr [rax+16]
    pop     qword ptr [rax+8]

    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    pop     rbp

    ret
DzSwitch ENDP

_TEXT   ENDS
END
