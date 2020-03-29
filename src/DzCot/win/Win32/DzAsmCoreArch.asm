;********************************************************************
;   @file       DzAsmCoreArch.asm
;   @brief      funciton used for switch cot in 32 bit windows
;   @author     Foreverflying <foreverflying@live.cn>
;   @date       2010/02/11
;********************************************************************

.386

;PUBLIC _CallDzCotEntry@0 PROC  ; CallDzCotEntry
;PUBLIC @DzSwitch@8 PROC        ; DzSwitch

_TEXT   SEGMENT

; void __stdcall CallDzCotEntry(void);
_CallDzCotEntry@0 PROC
    lea     edx, [esp+4]
    lea     ecx, [esp]
    push    edx
    push    ecx
    push    esi                 ; esi is pointer of host
    call    edi                 ; edi is pointer of function DzCotEntry
_CallDzCotEntry@0 ENDP

; void __fastcall DzSwitch(DzHost* host, DzCot* dzCot);
; host$ = ecx
; dzCot$ = edx
@DzSwitch@8 PROC
    push    ebp
    push    ebx
    push    esi
    push    edi

    push    dword ptr fs:[0]
    push    dword ptr fs:[4]
    push    dword ptr fs:[8]

    mov     esi, [ecx]          ; esi = host->currCot
    mov     [esi+4], esp        ; host->currCot->sp = esp
    mov     [ecx], edx          ; host->currCot = dzCot
    mov     esp, [edx+4]        ; esp = dzCot.sp

    pop     dword ptr fs:[8]
    pop     dword ptr fs:[4]
    pop     dword ptr fs:[0]

    pop     edi
    pop     esi
    pop     ebx
    pop     ebp

    ret
@DzSwitch@8 ENDP

_TEXT   ENDS
END
