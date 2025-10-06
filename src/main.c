#include "assert.h"
#include "vga.h"
#include "alloc.h"
#include "types.h"

extern void inf_loop();
extern void cli();

void kernel_entry() {
    clear_screen();
    //assert(42 == 37);
    for (u32 i = 0; i < 30 * 1024 * 64; i++) {
        void* addr = malloc_linear(1648, i % 64);
        printf("Address: %x\n", addr);
    }
    inf_loop();
}
