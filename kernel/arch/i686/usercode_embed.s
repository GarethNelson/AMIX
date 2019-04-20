[GLOBAL default_usercode]
[GLOBAL default_usercode_end]
align 16
section .rodata
default_usercode:
	incbin "usercode.bin"
default_usercode_end:
