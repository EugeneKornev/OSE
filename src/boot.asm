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

mov eax, TSS
mov word [tss_desc + 2], ax
shr eax, 16
mov byte [tss_desc + 4], al
mov byte [tss_desc + 7], ah
mov ax, tss_segment
ltr ax

extern kernel_entry
call kernel_entry


global inf_loop
inf_loop:
    jmp inf_loop

global cli
cli:
    cli
    ;mov byte [0xb8000], 'G'
    ;jmp inf_loop
    ret

global sti
sti:
    sti
    ret


global lidt
lidt:
    mov eax, dword [esp + 4]
    lidt [eax]
    ret

extern universal_handler

global collect_context
collect_context:
    push ds
    push es
    push fs
    push gs
    pusha ; 11 slide

    cld ; clear direction flag

    mov eax, data_segment ; reset segment registers
    mov ds, eax ; 20 slide
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov ebx, esp
    sub esp, 4 ; stack alignment
    and esp, -16
    mov dword [esp], ebx ; store original stack, 33 slide

    call universal_handler
    mov esp, ebx
    popa
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 8 ; clear error code and interrupt vector
    iret


global restore_context
restore_context:
    mov esp, dword [esp + 4]
    popa
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 8
    iret

global eflags
eflags:
    pushfd
    pop eax
    ret


global get_esp
get_esp:
    mov eax, esp
    ret    


global syscall_expr
syscall_expr:
    mov eax, dword [esp + 4]
    int 0x30
    ret

global readFromPort
readFromPort:
    ; esp + 4 = port
    mov dx, word [esp + 4]
    in al, dx
    ret

global writeToPort
writeToPort:
    ; esp + 4 = port
    ; esp + 8 = data
    mov dx, word [esp + 4]
    mov al, byte [esp + 8]

    out dx, al

    push edi
    push esi

    mov edi, eax

    ; delay loop
    mov esi, 0x1 ; loop size

.loop:
    dec esi
    jz .continue
    xor eax, eax
    cpuid
    jmp .loop

.continue:
    mov eax, edi
    pop esi
    pop edi
    ret


global setup_reg
setup_reg:
    mov eax, 0
    mov ebx, 1
    mov ecx, 2
    mov edx, 3
    mov edi, 4
    mov esi, 5
    mov ebp, 6
    ret


global not_allowed
not_allowed:
    ;jmp inf_loop
    ;cli
    ;sti
    ;lidt [eax]
    ;lgdt [eax]
    ;ltr ax
    ;mov cr0, eax
    ;in al, dx
    ;out dx, al
    ret 

global div_zero
div_zero:
    idiv eax
    ret

global pseudo_syscall
pseudo_syscall:
    int 0x21
    ret


gdt_desc:
    dw 0x2f ; = 47 = 48 - 1 to contain all 65536 descriptors; GDT limit
    dd gdt ; linear address

global code_segment_desc ; for 9 experiment

align 8
gdt:
    .null: dq 0 ; null descriptor
    code_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type db 0b1001_1010 ; P(1) - segment present flag; DPL(2) - descriptor privilege level; S(1) - descriptor
                        ; type flag (0 for data/code, 1 for ...); Type(4) - type of segment, highest bit: 0 -> data | 1 -> code: exec_flag, direction,
                        ; write_enable, accessed_flag
                        .G_B_0_AVL_limit_high: db 0b1100_1111 ; G(1) - granularity flag, ; B(1) - =1, uses with Type; L_flag(1) = 0 - ...; AVL(1) -
                        ; available bit
                        .base_high: db 0
    data_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type: db 0b1001_0010
                        .G_B_0_AVL_limit_high: db 0b1100_1111
                        .base_high: db 0
    user_code_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type: db 0b1111_1010 ; #11#_1####
                        .G_B_0_AVL_limit_high: db 0b1100_1111
                        .base_high: db 0
    user_data_segment_desc:
                        .limit_low: dw 0xff
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type: db 0b1111_0010 ; #11#_0###
                        .G_B_0_AVL_limit_high: db 0b1100_1111
                        .base_high: db 0
    tss_desc:           ; 14.64; 3036
                        .limit_low: dw  0x6b ; 107 = 108 - 1
                        .base_low: dw 0
                        .base_mid: db 0
                        .P_DPL_S_type: db 0b1000_1001
                        .G_0_0_AVL_limit_high: db 0
                        .base_high: db 0


tss_segment equ (tss_desc - gdt)

                    
TSS: ; 3034
    .previous_task_link: dd 0
    .esp0:               dd 0x7c00
    .ss0:                dw data_segment
    .reserved0:          dw 0
    .esp1:               dd 0
    .ss1:                dw 0
    .reserved1:          dw 0
    .esp2:               dd 0
    .ss2:                dw 0
    .reserved2:          dw 0
    .cr3:                dd 0
    .eip:                dd 0
    .eflags:             dd 0
    .eax:                dd 0
    .ecx:                dd 0
    .edx:                dd 0
    .ebx:                dd 0
    .esp:                dd 0
    .ebp:                dd 0
    .esi:                dd 0
    .edi:                dd 0
    .es:                 dw 0
    .reserved3:          dw 0
    .cs:                 dw 0
    .reserved4:          dw 0
    .ss:                 dw 0
    .reserved5:          dw 0
    .ds:                 dw 0
    .reserved6:          dw 0
    .fs:                 dw 0
    .reserved7:          dw 0
    .gs:                 dw 0
    .reserved8:          dw 0
    .ldt_selector:       dw 0
    .reserved9:          dw 0
    .debug_trap:         dw 0
    .io_map_base:        dw 108
    ;.tss_size:           dw 108
    .ssp:                dd 0
    
                            
    

                        
                        
                        
                        


code_segment equ 0x08
data_segment equ 0x10
; 15 - 3: index in gdt; 2: TI (0 for gdt); 1 - 0: PL(privilege level -> 0)

times 510-($-$$) db 0
dw 0xAA55