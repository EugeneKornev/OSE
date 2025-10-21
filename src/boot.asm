[BITS 16]

%define sectors ((N / 512) + ((N % 512) != 0))

cli
xor ax, ax ; setting stack
mov ss, ax
mov sp, 0x7c00

mov ax, 0x7e0 ; setting es 
mov es, ax
xor bx, bx
mov ds, bx

mov di, sectors

xor ch, ch
xor dh, dh
mov cl, 2

.loop:

mov ax, 0x201

int 0x13

jc $

mov ax, es
add ax, 0x20 ; = 32 = 512 / 16
mov es, ax
xor bx, bx

inc cl
cmp cl, 18 ; sectors count on floppy
jbe .check_sectors

mov cl, 1
inc dh
cmp dh, 1
jbe .check_sectors

xor dh, dh
inc ch

.check_sectors:
dec di
jnz .loop


lgdt [gdt_desc] ; GDTR = gdt_desc, pseudo descriptor
cld ; clear direction flag, df = 0; value fixed in ABI

mov eax, cr0 ; control register
or eax, 1
mov cr0, eax

jmp code_segment:tramp ; recommended

[BITS 32]
tramp: ;s setting data registers
mov eax, data_segment
mov ds, eax
mov ss, eax
mov es, eax
mov fs, eax
mov gs, eax

extern kernel_entry
call kernel_entry


global inf_loop
inf_loop:
    jmp inf_loop

global cli
cli:
    cli
    ret


gdt_desc:
    dw 0x17 ; = 23 = 24 - 1 to contain all 65536 descriptors; GDT limit
    dd gdt ; linear address


align 8
gdt:
    .null: dq 0 ; null descriptor
    code_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type db 0b10011010 ; P(1) - segment present flag; DPL(2) - descriptor privilege level; S(1) - descriptor
                        ; type flag (0 for data/code, 1 for ...); Type(4) - type of segment, highest bit: 0 -> data | 1 -> code: exec_flag, direction,
                        ; write_enable, accessed_flag
                        .G_B_0_AVL_limit_high: db 0b11001111 ; G(1) - granularity flag, ; B(1) - =1, uses with Type; L_flag(1) = 0 - ...; AVL(1) -
                        ; available bit
                        .base_high: db 0
    data_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type: db 0b10010010
                        .G_B_0_AVL_limit_high: db 0b1100_1111
                        .base_high: db 0


code_segment equ 0x08
data_segment equ 0x10
; 15 - 3: index in gdt; 2: TI (0 for gdt); 1 - 0: PL(privilege level -> 0)

times 510-($-$$) db 0
dw 0xAA55