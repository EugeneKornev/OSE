#include "interrupts.h"
#include "types.h"
#include "panic.h"
#include "assert.h"
#include "alloc.h"
#include "vga.h"

static u32 tramp_size = 8;
static u32 vectors_count = 256;
static u8 int_with_error_code[8] = {0x8, 0xA, 0xB, 0xC, 0xD, 0xE, 0x11, 0x15}; // 2974

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



static void* gen_idt() {
    u8* tramps = malloc_linear(tramp_size * vectors_count, 8);
    
    for (u16 vector = 0; vector < vectors_count; vector++) {
        // push eax - fake error code, if needed
        // push vector
        // jmp collect_context
        // 1 + 1 + 1 + 1 + 4 = push eax + push + vector + jmp + address = 8

        bool has_error_code = false;
        for (u8 i = 0; i < 8; i++) {
            if (int_with_error_code[i] == vector) {
                has_error_code = true;
            }
        }

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
        idt[vector].gate_desc_type = 0b110; // interrupt gate 
        idt[vector].fixed = 0b01; // D = 1 and fixed 0
        idt[vector].dpl = 0b00;
        idt[vector].P = 0b1; // present bit = 1
        idt[vector].offset_high = ((u32)(tramps + tramp_size * vector) >> 16) & 0xFFFF;
      }

    return idt;
}
    
void setup_interrupts() {
    void* idt = gen_idt();
    u16 idt_limit = vectors_count * sizeof(interrupt_desc) - 1;
    u64 pseudo_idt = ((u64)idt << 16) | idt_limit;
    lidt(&pseudo_idt);
}

void universal_handler(struct interrupt_context* context) {
    panic("unhandled interrupt #%x at %x:%x\n\n"
          "Registers: \n"
          "    EAX: %x" "    EBX: %x" "    ECX: %x" "    EDX: %x\n"
          "    EDI: %x" "    ESI: %x" "    ESP: %x" "    EBP: %x\n"
          "    DS : %x" "    ES : %x" "    GS : %x" "    FS : %x\n\n"
          "Error code: %x\n\n"
          "EFLAGS: %x\n", context->int_vector, context->cs, context->eip, context->eax, context->ebx, context->ecx, context->edx,
          context->edi, context->esi, context->esp, context->ebp, context->ds, context->es, context->gs, context->fs, context->error_code,
          context->eflags);
}