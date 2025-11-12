#include "alloc.h"
#include "assert.h"
#include "interrupts.h"
#include "panic.h"
#include "types.h"
#include "vga.h"

void kernel_entry() {
    
    clear_screen();
    set_fg_color(0xf);
 
    setup_interrupts();
    setup_reg();

    //div_zero();
    pseudo_syscall();
    //sti();
    
    inf_loop();
}
