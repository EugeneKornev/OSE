#include "interrupts.h"
#include "types.h"
#include "panic.h"
#include "assert.h"
#include "alloc.h"
#include "vga.h"
#include "IO.h"

static u32 tramp_size = 8;
static u32 vectors_count = 256;
static u8 int_with_error_code[10] = {0x8, 0xA, 0xB, 0xC, 0xD, 0xE, 0x11, 0x15, 0x1D, 0x1E}; // 2974

static u16 master_command_port = 0x20;
static u16 master_data_port = 0x21;
static u16 slave_command_port = 0xA0;
static u16 slave_data_port = 0xA1;

extern void collect_context(); // in boot.asm

typedef struct {
    u16 offset_low;
    u16 segment_selector;
    u8 reserved;
    u8 gate_desc_type: 3;
    u8 fixed: 2; // D = 1 & fixed 0
    u8 dpl: 2;
    u8 P: 1; // present bit
    u16 offset_high;
} interrupt_desc;


struct interrupt_context {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    alignas(4) u16 gs, fs, es, ds;
    alignas(4) u8 int_vector; // made by trampoline
    u32 error_code;
    u32 eip;
    alignas(4) u16 cs;
    u32 eflags;
};


static bool vector_has_error_code(u8 vector) {
    for (u8 i = 0; i < 10; i++) {
        if (int_with_error_code[i] == vector) {
            return true;
        }
    }
    return false;
}


static void* gen_idt() {
    u8* tramps = malloc_linear(tramp_size * vectors_count, 8);
    
    for (u16 vector = 0; vector < vectors_count; vector++) {
        // push eax - fake error code, if needed
        // push vector
        // jmp collect_context
        // 1 + 1 + 1 + 1 + 4 = push eax + push + vector + jmp + address = 8

        bool has_error_code = vector_has_error_code(vector);

        u8* tramp = tramps + vector * tramp_size;
        u32 offset = 0;
        if (!has_error_code) {
            tramp[offset++] = 0x50; // push eax
        }
        tramp[offset++] = 0x6A; // push ... (const)
        tramp[offset++] = vector; // const ^
        tramp[offset++] = 0xE9; //jmp near (not short) ... https://www.felixcloutier.com/x86/jmp
        u32 offset_to_func = (u32)collect_context - (u32)(tramp + offset + 4); // enf of jmp instruction
        *(u32*)(tramp + offset) = offset_to_func;
    }

    interrupt_desc* idt = malloc_linear(sizeof(interrupt_desc) * vectors_count, sizeof(interrupt_desc)); // slide 37

    for (u16 vector = 0; vector < vectors_count; vector++) {
        idt[vector].offset_low = (u32)(tramps + tramp_size * vector) & 0xFFFF;
        idt[vector].segment_selector = 0x08;
        idt[vector].reserved = 0;
        idt[vector].gate_desc_type = 0b110; // interrupt gate = 0b110, trap gate = 0b111;
        idt[vector].fixed = 0b01;           // D = 1 and fixed 0
        idt[vector].dpl = 0b00;
        idt[vector].P = 0b1; // present bit = 1
        idt[vector].offset_high = ((u32)(tramps + tramp_size * vector) >> 16) & 0xFFFF;
    }

    //idt[0x20].gate_desc_type = 0b111; // set timer handler to trap gate
    //idt[0x21].gate_desc_type = 0b111; // set keyboard handler to trap gate
    return idt;
}
    
void* setup_interrupts() {
    void* idt = gen_idt();
    u16 idt_limit = vectors_count * sizeof(interrupt_desc) - 1;
    u64 pseudo_idt = ((u64)idt << 16) | idt_limit;
    lidt(&pseudo_idt);
    return idt;
}

void set_master_mask(u8 mask) {
    writeToPort(master_data_port, ~mask);
}

void setup_Intel8259(bool autoEOI) {
    // ICW 1
    // port = command port
    // optional = no
    // main flags and number of add words
    // D0 = 1 -> there will be ICW 4
    // D1 = 0 -> there will be cascas
    // D3(Level/Edge triggered mode) = 0 -> edge
    // D4 = 1 -> start of setting 8259
    // D2, D5-D7 - bits for MCS-80/85

    // ICW 2
    // port = data port
    // optional = no
    // mapping for IRQs on vectors

    // ICW 3
    // port = data port
    // optional = yes
    // cascade configuration
    // if ICW_1.D1 == 0 -> cascade need to be configurated
    // for master: mask = pin to which slave connected
    // for slave: mask = pin that slave connected to master

    // ICW 4
    // port = data port
    // optional = yes
    // D0, D2-D4 = (1, _, 0, 0, 0) -> fully nested mode
    // D1(auto EOI)
    // D5-D7 - reserved
    
    // disable all interrupts
    writeToPort(master_data_port, 0xff);
    writeToPort(slave_data_port, 0xff);

    u8 icw_1 = 0b10001; // = 0x11
    // icw 1
    writeToPort(master_command_port, icw_1);
    writeToPort(slave_command_port, icw_1);

    // icw 2; 13:64
    writeToPort(master_data_port, 0x20); 
    writeToPort(slave_data_port, 0x28);


    // icw 3
    writeToPort(master_data_port, 1 << 2); // mask for master of slave
    writeToPort(slave_data_port, 2); // pin that slave is connected to


    // icw 4
    writeToPort(master_data_port, (u8) autoEOI << 1 | 1);
    writeToPort(slave_data_port, (u8) autoEOI << 1 | 1);
}

void eoi() {
    // send EOI signal to master 8259
    writeToPort(master_command_port, 0x20); // 13:84
    // as we ignored slave 8259, we send EOI only to master 8259
}

int global_counter = 0;

#define N 136


static void delay_for_experiment() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 50000; j++) {
            writeToPort(0x80, 0x80);
        }
        printf("%d ", i);
    }
    printf("\n");
}


void timer_handler(struct interrupt_context* context) {
    //panic("unhandled interrupt #%x at %x:%x\n\n"
    //              "Registers: \n"
    //              "    EAX: %x" "    EBX: %x" "    ECX: %x" "    EDX: %x\n"
    //              "    EDI: %x" "    ESI: %x" "    ESP: %x" "    EBP: %x\n"
    //              "    DS : %x" "    ES : %x" "    GS : %x" "    FS : %x\n\n"
    //              "Error code: %x\n\n"
    //              "EFLAGS: %x\n", context->int_vector, context->cs, context->eip, context->eax, context->ebx, context->ecx, context->edx,
    //              context->edi, context->esi, context->esp, context->ebp, context->ds, context->es, context->gs, context->fs, context->error_code,
    //              context->eflags);
    //
    //printf("%d ", global_counter++);

    //set_master_mask(2); // mask timer in PIC

    //delay_for_experiment();
    //sti();
    //delay_for_experiment();

    //if (global_counter < N) {
        //eoi();
        //sti();
    //}
    //
    //eoi();
    //sti();
    //inf_loop();
    //
    //global_counter = 0;
    return;
}

void keyboard_handler(struct interrupt_context* context) {
    //panic("unhandled interrupt #%x at %x:%x\n\n"
    //              "Registers: \n"
    //              "    EAX: %x" "    EBX: %x" "    ECX: %x" "    EDX: %x\n"
    //              "    EDI: %x" "    ESI: %x" "    ESP: %x" "    EBP: %x\n"
    //              "    DS : %x" "    ES : %x" "    GS : %x" "    FS : %x\n\n"
    //              "Error code: %x\n\n"
    //              "EFLAGS: %x\n", context->int_vector, context->cs, context->eip, context->eax, context->ebx, context->ecx, context->edx,
    //              context->edi, context->esi, context->esp, context->ebp, context->ds, context->es, context->gs, context->fs, context->error_code,
    //              context->eflags);\
    //
    u8 read_byte = readFromPort(0x60);
    //printf("%c ", read_byte);
    printf("%x ", read_byte);
    //sti();
    //eoi();
    //inf_loop();
    return;
}

void universal_handler(struct interrupt_context* context) {
    switch (context->int_vector) {
        case 0x20:
            timer_handler(context);
            break;
        case 0x21:
            keyboard_handler(context);
            break;
        default:
            panic("unhandled interrupt #%x at %x:%x\n\n"
                  "Registers: \n"
                  "    EAX: %x" "    EBX: %x" "    ECX: %x" "    EDX: %x\n"
                  "    EDI: %x" "    ESI: %x" "    ESP: %x" "    EBP: %x\n"
                  "    DS : %x" "    ES : %x" "    GS : %x" "    FS : %x\n\n"
                  "Error code: %x\n\n"
                  "EFLAGS: %x\n", context->int_vector, context->cs, context->eip, context->eax, context->ebx, context->ecx, context->edx,
                  context->edi, context->esi, context->esp, context->ebp, context->ds, context->es, context->gs, context->fs, context->error_code,
                  context->eflags);
            break;
    }
}