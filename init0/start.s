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


	mov esi, greeter_string
	call print_string

	mov eax,0
	int 0x80
	mov ebx,eax

	mov esi,fork_ret_string
	call print_string

	mov eax,ebx
	push eax
	mov eax,sys_debug_out_num
	int 0x80

	push 10
	mov eax, sys_debug_out
	int 0x80

	mov esi,my_tid_string
	call print_string

	mov eax,sys_get_tid
	int 0x80
	push eax
	mov eax, sys_debug_out_num
	int 0x80

loop:	jmp loop
	call init0_main


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
%define X(syscall_num,syscall_name) sys_ %+ syscall_name equ syscall_num
	%[%include "AMIX/syscalls.def"]
%undef X
