[GLOBAL _start]
[EXTERN init0_main]

_start:
 	mov esp,0x80024000

     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax ;we don't need to worry about SS. it's handled by iret
 
     mov eax,esp
     push 0x23 ;user data segment with bottom 2 bits set for ring 3
     push eax ;push our current stack just for the heck of it
     pushf
     push 0x1B; ;user code segment with bottom 2 bits set for ring 3
     push a ;may need to remove the _ for this to work right 
     iret
     a:

	call init0_main

loop:	jmp loop

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

greeter_string:          db 10,'[USERCODE]',9, 'Hello from init0',10,0
fork_ret_string:	 db 10,'Fork returned: ',0
my_tid_string:		 db 10,'My TID: ',0
%define X(syscall_num,syscall_name,params) sys_ %+ syscall_name equ syscall_num
	%[%include "AMIX/syscalls.def"]
%undef X
