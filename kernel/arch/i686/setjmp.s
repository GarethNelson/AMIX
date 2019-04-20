global flush_tss
flush_tss:
   mov ax, 0x2B      ; Load the index of our TSS structure - The index is
                     ; 0x28, as it is the 5th selector and each is 8 bytes
                     ; long, but we set the bottom two bits (making 0x2B)
                     ; so that it has an RPL of 3, not zero.
   ltr ax            ; Load 0x2B into the task state register.
   ret

global longjmp:function longjmp.end-longjmp
longjmp:
        mov     ecx, [esp+4]    ; First parameter = jmp_buf
        mov     eax, [esp+8]    ; Second = return value
        
        mov     esp, [ecx+0]
        mov     ebp, [ecx+4]
        mov     ebx, [ecx+12]
        mov     esi, [ecx+16]
        mov     edi, [ecx+20]
        
        mov     edx, [ecx+24]
        push    edx
        popf

        mov     ecx, [ecx+8]
        jmp     ecx
.end:
        
global setjmp:function setjmp.end-setjmp
setjmp:
        mov     ecx, [esp+4]    ; First parameter = jmp_buf
        ;; Stack pointer for longjmp should ignore setjmp's return address
        ;; i.e. esp+4.
        mov     eax, esp
        add     eax, 4
        mov     [ecx+0], eax
        mov     [ecx+4], ebp

        mov     eax, [esp]      ; Return address at [esp]
        mov     [ecx+8], eax

        mov     [ecx+12], ebx
        mov     [ecx+16], esi
        mov     [ecx+20], edi

        pushf                   ; EFLAGS
        pop     eax
        mov     [ecx+24], eax

        xor     eax, eax        ; Return 0
        ret
.end:
