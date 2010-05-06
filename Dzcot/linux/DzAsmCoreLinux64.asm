;DzAsmCoreWin.asm: funciton used for switch cot

;PUBLIC	DzSwitch PROC    ; DzSwitch
EXTRN	DzcotRoutine:PROC

_TEXT   SEGMENT

; void CallDzcotRoutine
CallDzcotRoutine PROC   ;CallDzcotRoutine
    mov rcx, [esp+8]
    mov rdx, [esp+16]
    jmp DzcotRoutine
CallDzcotRoutine ENDP   ;CallDzcotRoutine

; void DzSwitch( DzHost *host, DzThread *dzThread );
; host$ = ecx
; dzThread$ = edx
DzSwitch PROC   ; DzSwitch
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

    mov rsi, [rcx]      ;esi = host->currThread
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

    ret 0
DzSwitch ENDP   ; DzSwitch

_TEXT   ENDS
END