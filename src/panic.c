#include "panic.h"
#include "vga.h"

extern void cli();
extern void inf_loop();

void vpanic(const char* format, va_list args) {
    cli();
    clear_screen();
    //set_bg_color(0xf);
    set_fg_color(0xc);
    printf("panic: ");
    vprintf(format, args);
    inf_loop();
}

void panic(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vpanic(format, args);
    va_end(args);
}