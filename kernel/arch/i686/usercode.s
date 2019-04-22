; this is the userland code that inits everything as appropriate



[GLOBAL start]
[BITS 32]
[ORG 0x80000000]
[MAP all usercode.map]


start:

	mov esp,0x80024000

	mov esi, greeter_string
	call print_string

	mov eax, sys_exit
	int 0x80


endless_loop:
 	jmp endless_loop

print_string:
	.run:
	lodsb
	cmp al,0
	je .done
	push eax
	mov eax, sys_debug_out
	int 0x80
	pop ecx
	jmp .run
	.done:
	ret

greeter_string:          db 10,'[USERCODE]',9, 'Hello from default usercode',10,0

%define X(syscall_num,syscall_name,params) sys_ %+ syscall_name equ syscall_num
	%[%include "syscalls.def"]
%undef X

