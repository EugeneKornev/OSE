[BITS 16]
[ORG 0x7c00]

%define sectors ((N / 512) + ((N % 512) != 0))

mov [boot_drive], dl

cli
xor ax, ax ; setting stack
mov ss, ax
mov sp, 0x7c00

mov ax, 0x7e0 ; setting es 
mov es, ax
xor bx, bx
mov ds, bx

mov word [sectors_count], sectors

mov [cylinder], byte 0
mov [head], byte 0
mov [sector], byte 2

.loop:

mov ah, 0x2
mov al, 1
mov ch, [cylinder]
mov cl, [sector]
mov dh, [head]
mov dl, [boot_drive]

int 0x13

jc .error

mov ax, es
add ax, 0x20 ; = 32 = 512 / 16
mov es, ax
xor bx, bx

inc byte [sector]
cmp byte [sector], 18 ; sectors count on floppy
jbe .check_sectors

mov byte [sector], 1
inc byte [head]
cmp byte [head], 2
jb .check_sectors

mov byte [head], 0
inc byte [cylinder]

.check_sectors:
dec word [sectors_count]
jnz .loop

mov byte [cylinder], 0
mov byte [head], 0
mov byte [sector], 0

.inf_loop:
jmp $

jmp 0x7e00:0x0

.error:
jmp $

boot_drive: db 0
sectors_count: dw 0
cylinder: db 0
head: db 0
sector: db 0

times 510-($-$$) db 0
dw 0xAA55