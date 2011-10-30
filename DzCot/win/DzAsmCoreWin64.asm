;********************************************************************
;    created:    2010/02/11 22:07
;    file:       DzAsmCoreWin64.asm
;    author:     Foreverflying
;    purpose:    funciton used for switch cot on win64
;********************************************************************

;PUBLIC CallDzCotEntry PROC     ; CallDzCotEntry
;PUBLIC DzSwitch PROC           ; DzSwitch

EXTRN   DzCotEntry : PROC

_TEXT   SEGMENT

; void CallDzCotEntry( void );
CallDzCotEntry PROC     ;CallDzCotEntry
    lea     r8, [rsp+16]
    lea     rdx, [rsp+8]
    mov     rcx, [rsp]
    sub     rsp, 32
    call    DzCotEntry
CallDzCotEntry ENDP     ;CallDzCotEntry

; void DzSwitch( DzHost* host, DzCot* dzCot );
; host$ = rcx
; dzCot$ = rdx
DzSwitch PROC           ; DzSwitch
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

    mov     rsi, [rcx]      ;rsi = host->currCot
    mov     [rsi+8], rsp    ;host->currCot->sp = rsp
    mov     [rcx], rdx      ;host->currCot = dzCot
    mov     rsp, [rdx+8]    ;rsp = dzCot.sp

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
DzSwitch ENDP           ; DzSwitch

_TEXT   ENDS
END
