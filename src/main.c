#include "assert.h"
#include "vga.h"
#include "alloc.h"
#include "types.h"
#include "panic.h"
#include "interrupts.h"

void kernel_entry() {
    
    clear_screen();
    set_fg_color(0xf);
 
    setup_interrupts();
    setup_reg();

    div_zero();
    //pseudo_syscall();
    //sti();
    
    inf_loop();
}
