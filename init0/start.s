[GLOBAL _start]
[EXTERN init0_main]

_start:
 	mov esp,0x80024000

	call init0_main

loop:	jmp loop

