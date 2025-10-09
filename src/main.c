#include "assert.h"
#include "vga.h"
#include "alloc.h"
#include "types.h"
#include "panic.h"

void kernel_entry() {
    clear_screen();
    //assert(42 == 37);
    for (u32 i = 0; i < 30 * 1024 * 64; i++) {
        void* addr = malloc_linear(1648, i % 64);
        if (((u32) addr % ((i % 64 == 0) ? 1 : (i % 64))) != 0) {
            panic("Alignment is wrong\n");
        } else {
            printf("Address is %x and alignment is right\n", addr);
        }
    }
    inf_loop();
}
