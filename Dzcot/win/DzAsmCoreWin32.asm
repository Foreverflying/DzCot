;********************************************************************
;    created:    2010/02/11 22:07
;    file:       DzAsmCoreWin32.asm
;    author:     Foreverflying
;    purpose:    funciton used for switch cot on win32
;********************************************************************

    .386

;PUBLIC _CallDzcotEntry@0 PROC  ; CallDzcotEntry
;PUBLIC @DzSwitch@8 PROC        ; DzSwitch

EXTRN   _DzcotEntry@8 : PROC

_TEXT   SEGMENT

; void __stdcall CallDzcotEntry( void );
_CallDzcotEntry@0 PROC  ;CallDzcotEntry
    lea     eax, [esp+4]
    mov     edx, esp
    push    eax
    push    edx
    call    _DzcotEntry@8
_CallDzcotEntry@0 ENDP  ;CallDzcotEntry

; void __fastcall DzSwitch( DzHost* host, DzThread* dzThread );
; host$ = ecx
; dzThread$ = edx
@DzSwitch@8 PROC        ; DzSwitch
    push    ebp
    push    ebx
    push    esi
    push    edi

    push    dword ptr fs:[0]
    push    dword ptr fs:[4]
    push    dword ptr fs:[8]

    mov     esi, [ecx]      ;esi = host->currThread
    mov     [esi+4], esp    ;host->currThread->sp = esp
    mov     [ecx], edx      ;host->currThread = dzThread
    mov     esp, [edx+4]    ;esp = dzThread.sp

    pop     dword ptr fs:[8]
    pop     dword ptr fs:[4]
    pop     dword ptr fs:[0]

    pop     edi
    pop     esi
    pop     ebx
    pop     ebp

    ret
@DzSwitch@8 ENDP        ; DzSwitch

_TEXT   ENDS
END
