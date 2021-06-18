    .code16
    .data

gdt:
	.quad	0x00

code_segment:
	.word	0xFFFF
	.word	0x00
	.byte	0x00
	.byte	0x9a
	.byte	0xcf
	.byte	0x00
	
data_segment:
	.word	0xFFFF
	.word	0x00
	.byte	0x00
	.byte	0x92
	.byte	0xcf
    .byte   0x00

    .global gdt_descriptor
gdt_descriptor:
	.word	(gdt_descriptor - gdt - 1)
	.long	(gdt)
    