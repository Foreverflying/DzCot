;********************************************************************
;    created:    2010/02/11 22:07
;    file:       DzAsmCoreWin.asm
;    author:     Foreverflying
;    purpose:    funciton used for switch cot on win32
;********************************************************************

	.386

;PUBLIC	@DzSwitch@8 PROC    ; DzSwitch

_TEXT   SEGMENT

; void __fastcall DzSwitch( DzHost *host, DzThread *dzThread );
; host$ = ecx
; dzThread$ = edx
@DzSwitch@8 PROC    ; DzSwitch
    push ebp
    push ebx
    push esi
    push edi

    push dword ptr fs:[0]
    push dword ptr fs:[4]
    push dword ptr fs:[8]

    mov esi, [ecx]      ;esi = host->currThread
    mov [esi+4], esp    ;host->currThread->esp = esp
    mov [ecx], edx      ;host->currThread = dzThread
    mov esp, [edx+4]    ;esp = dzThread.esp

    pop dword ptr fs:[8]
    pop dword ptr fs:[4]
    pop dword ptr fs:[0]

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret 0
@DzSwitch@8 ENDP    ; DzSwitch

_TEXT   ENDS
END