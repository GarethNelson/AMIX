[GLOBAL _start]
[EXTERN main]

_start:
 	mov esp,stack

	call main

loop:	jmp loop

stack_bottom:
	resb 0x4000
stack:
