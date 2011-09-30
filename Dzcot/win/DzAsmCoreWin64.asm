;********************************************************************
;    created:    2010/02/11 22:07
;    file:       DzAsmCoreWin64.asm
;    author:     Foreverflying
;    purpose:    funciton used for switch cot on win64
;********************************************************************

;PUBLIC DzSwitchFast PROC    ; DzSwitchFast
EXTRN   DzcotRoutine:PROC

_TEXT   SEGMENT

; void CallDzcotRoutine
CallDzcotRoutine PROC   ;CallDzcotRoutine
    mov rcx, [rsp+8]
    mov rdx, [rsp+16]
    jmp DzcotRoutine
CallDzcotRoutine ENDP   ;CallDzcotRoutine

; void DzSwitchFast( DzHost* host, DzThread* dzThread );
; host$ = rcx
; dzThread$ = rdx
DzSwitchFast PROC   ; DzSwitchFast
    push rbp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    mov rax, gs:[30h]
    push qword ptr [rax+8]
    push qword ptr [rax+16]

    mov rsi, [rcx]      ;rsi = host->currThread
    mov [rsi+8], rsp    ;host->currThread->sp = rsp
    mov [rcx], rdx      ;host->currThread = dzThread
    mov rsp, [rdx+8]    ;rsp = dzThread.sp

    pop qword ptr [rax+16]
    pop qword ptr [rax+8]

    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp

    ret
DzSwitchFast ENDP   ; DzSwitchFast

_TEXT   ENDS
END
