;********************************************************************
;    created:    2010/02/11 22:07
;    file:       DzAsmCoreWin32.asm
;    author:     Foreverflying
;    purpose:    funciton used for switch cot on win32
;********************************************************************

    .386

;PUBLIC _CallDzCotEntry@0 PROC  ; CallDzCotEntry
;PUBLIC @DzSwitch@8 PROC        ; DzSwitch

EXTRN   _DzCotEntry@12 : PROC

_TEXT   SEGMENT

; void __stdcall CallDzCotEntry( void );
_CallDzCotEntry@0 PROC  ;CallDzCotEntry
    lea     edx, [esp+8]
    lea     ecx, [esp+4]
    mov     eax, [esp]
    push    edx
    push    ecx
    push    eax
    call    _DzCotEntry@12
_CallDzCotEntry@0 ENDP  ;CallDzCotEntry

; void __fastcall DzSwitch( DzHost* host, DzCot* dzCot );
; host$ = ecx
; dzCot$ = edx
@DzSwitch@8 PROC        ; DzSwitch
    push    ebp
    push    ebx
    push    esi
    push    edi

    push    dword ptr fs:[0]
    push    dword ptr fs:[4]
    push    dword ptr fs:[8]

    mov     esi, [ecx]      ;esi = host->currCot
    mov     [esi+4], esp    ;host->currCot->sp = esp
    mov     [ecx], edx      ;host->currCot = dzCot
    mov     esp, [edx+4]    ;esp = dzCot.sp

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
