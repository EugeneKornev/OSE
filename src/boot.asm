[BITS 16]
[ORG 0x7c00]

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

jc .error

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


.inf_loop:
jmp $

jmp 0x7e00:0x0

.error:
jmp $

times 510-($-$$) db 0
dw 0xAA55