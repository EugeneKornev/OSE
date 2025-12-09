#pragma once
#include "types.h"

//struct interrupt_context;
struct interrupt_context {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    alignas(4) u16 gs, fs, es, ds;
    alignas(4) u8 int_vector; // made by trampoline
    u32 error_code;
    u32 eip;
    alignas(4) u16 cs;
    u32 eflags;
};


void* setup_interrupts();

void universal_handler(struct interrupt_context* context);

void setup_Intel8259(bool autoEOI);

void set_master_mask(u8 mask);

extern int global_counter;
