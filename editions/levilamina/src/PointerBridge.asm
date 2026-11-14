; Preserve the game's caller for a chained hook's _ReturnAddress() admission.
; Windows x64: entry RSP is 8 mod 16; 58h reserves shadow space and three
; argument slots while aligning the observer call. No nonvolatile register
; is changed. The unwind description also covers observer exceptions.
EXTERN MMB_PointerObserver:PROC
EXTERN MMB_PointerOriginal:QWORD
PUBLIC PointerHook

.code
PointerHook PROC FRAME
    sub rsp, 58h
    .allocstack 58h
    .endprolog
    mov [rsp+20h], rcx
    mov [rsp+28h], rdx
    mov [rsp+30h], r8
    mov r9, [rsp+58h]
    call MMB_PointerObserver
    test al, al
    jnz consumed
    mov rcx, [rsp+20h]
    mov rdx, [rsp+28h]
    mov r8, [rsp+30h]
    add rsp, 58h
    jmp QWORD PTR [MMB_PointerOriginal]
consumed:
    mov eax, 1
    add rsp, 58h
    ret
PointerHook ENDP
END
