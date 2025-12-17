#include "alloc.h"
#include "assert.h"
#include "interrupts.h"
#include "panic.h"
#include "types.h"
#include "vga.h"
#include "userspace.h"

void inf_loop_with_inc() {
    //global_counter = 1;
    for (;;) {
        printf("%d ", global_counter++);
    }
}

void inf_loop_by_syscall() {
    for (;;) {
        syscall_expr(global_counter++);
    }
}

void user_program() {
    //printf("also here\n");
    //printf("%d ", get_esp());
    //inf_loop_with_inc();
    //not_allowed();
    //code_segment_desc &= ~((u64)1 << 47);
    //inf_loop();
    inf_loop_by_syscall();
}

void kernel_entry() {
    
    clear_screen();
    set_fg_color(0xf);
 
    setup_interrupts();
    setup_Intel8259(true); // auto EOI = true/false


    //set_master_mask(0); // disable all
    set_master_mask(1); // setup timer
    //set_master_mask(2); // setup keyboard
    //set_master_mask(3); // setup timer and keyboard

    u8* stack = malloc_linear(4096, 16) + 4096;
    start_process(user_program, stack);
    
    //sti();
    //printf("after sti");
    //inf_loop();
    //inf_loop_with_inc();
}
