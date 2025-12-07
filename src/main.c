#include "alloc.h"
#include "assert.h"
#include "interrupts.h"
#include "panic.h"
#include "types.h"
#include "vga.h"

void inf_loop_with_inc() {
    for (;;) {
        printf("%d ", global_counter++);
    }
}

void kernel_entry() {
    
    clear_screen();
    set_fg_color(0xf);
 
    setup_interrupts();
    setup_Intel8259(true); // auto EOI = true/false
    
    //set_master_mask(1); // setup timer
    set_master_mask(2); // setup keyboard
    //set_master_mask(3); // setup timer and keyboard
    
    sti();
    //printf("after sti");
    inf_loop();
    //inf_loop_with_inc();
}
