#include "userspace.h"
#include "interrupts.h"
#include "types.h"

#include "vga.h"

typedef struct {
    struct interrupt_context context;
    u32 esp;
    alignas(4) u16 ss;
} user_context;


void start_process(void* code_entry, void* stack) {
    user_context u_context;
    u_context.context.cs = 0x1b; // user_code_segment | CPL = 3
    u_context.context.ds = 0x23; // user_data_segment | RPL = 3
    u_context.context.es = 0x23;
    u_context.context.fs = 0x23;
    u_context.context.gs = 0x23;
    u_context.context.eip = (u32) code_entry;
    u_context.context.eflags = (eflags() & (~((u32)0b11 << 12))) | 1 << 9; // IOPL = 0, IF = 1
    u_context.ss = 0x23;
    u_context.esp = (u32) stack;
    //printf("here\n");
    //inf_loop();
    restore_context(&u_context);
}