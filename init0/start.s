[GLOBAL _start]
[EXTERN init0_main]

_start:
 	mov esp,stack

	call init0_main

loop:	jmp loop

stack_bottom:
	resb 0x4000
stack:
