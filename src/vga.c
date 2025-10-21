#include "vga.h"
#include "assert.h"
#include "alloc.h"
#include "panic.h"
#include <stdbool.h>


#define UPDATE_VGA_FIELD(x, y, MASK, SHIFT, VALUE) { \
    u16 temp_char = *(buffer_start + (y) * columns + (x)); \
    temp_char = (temp_char & ~(MASK)) | ((VALUE) << (SHIFT)); \
    *(buffer_start + (y) * columns + (x)) = temp_char; \
}
    

static u32 columns = 80;
static u32 rows = 25;
static u16* buffer_start = (u16*) 0xB8000;
static u32 cur_x = 0;
static u32 cur_y = 0;


static inline void put_char(u8 c, u32 x, u32 y) {
    UPDATE_VGA_FIELD(x, y, 0xff, 0, c);
}

static inline void put_fg_color(u8 color, u32 x, u32 y) {
    UPDATE_VGA_FIELD(x, y, 0xf00, 8, color);
}

static inline void put_bg_color(u8 color, u32 x, u32 y) {
    UPDATE_VGA_FIELD(x, y, 0xf000, 12, color);
}

static void scroll_down() {
    memmove(buffer_start, buffer_start + columns, columns * (rows - 1) * 2);
    for (u32 i = 0; i < columns; i++) {
        put_char(0, i, rows - 1);
    }
}

void clear_screen() {
    for (u32 i = 0; i < columns; i++) {
        for (u32 j = 0; j < rows; j++) {
            put_char(0, i, j);
        }
    }
    cur_x = 0;
    cur_y = 0;
}

void set_bg_color(u8 color) {
    for (u32 i = 0; i < columns; i++)  {
        for (u32 j = 0; j < rows; j++) {
            put_bg_color(color, i, j);
        }
    }
}

void set_fg_color(u8 color) {
    for (u32 i = 0; i < columns; i++) {
        for (u32 j = 0; j < rows; j++) {
            put_fg_color(color, i, j);
        }
    }
}

static void check_screen() {
    if (cur_x >= columns) {
        cur_y += 1;
        cur_x = 0;
    }
    if (cur_y >= rows) {
        scroll_down();
        cur_y -= 1;
    }
}

static void print_char(char c) {
    if (c == '\n') {
        cur_y += 1;
        cur_x = 0;
    } else if (c == '\r') {
        cur_x = 0;
    } else {
        put_char(c, cur_x, cur_y);
        cur_x += 1;
    }
    check_screen();
}

static void print_string(const char* str) {
    for (const char* c = str; *c; c++) {
        print_char(*c);
    }
}


static void print_u(u32 n, u8 base) {
    if (n == 0) {
        print_char('0');
        return;
    }
    
    char buffer[32];
    for (u8 k = 0; k < 32; k++) {
        buffer[k] = 0;
    }
    
    u8 i = 0;
    char temp_char;
    while (n > 0) {
        u32 digit = n % base;
        if (digit < 10) {
            temp_char = '0';
        } else {
            temp_char = 'a';
            digit -= 10;
        }
        buffer[i] = temp_char + digit;
        i++;
        n /= base;
    }

    if (base == 16) {
        for (u8 j = 8; j-- > 0;) {
            if (!buffer[j]) {
                print_char('0');
            } else {
                print_char(buffer[j]);
            }
        }
    } else {
        for (u8 j = 32; j-- > 0; ){
            if (!buffer[j]) {
                continue;
            }
            print_char(buffer[j]);
        }
    }
}

static void print_s(i32 n, u8 base) {
    if (n < 0) {
        print_char('-');
        n = -n;
    }
    print_u(n, base);
}

void vprintf(const char* format, va_list args) {
    bool cs = false;
    for (const char* c = format; *c; c++) {
        if (cs) {
            switch (*c) {
                case 'd': {
                        i32 n = va_arg(args, i32);
                        print_s(n, 10);
                        break;
                    }
                case 'x': {
                         print_string("0x");
                         i32 n = va_arg(args, u32);
                         print_u(n, 16);
                         break;
                    }
                case 'c': {
                        char c = va_arg(args, int);
                        print_char(c);
                        break;
                    }
                case 's': {
                        const char* str = va_arg(args, const char*);
                        print_string(str);
                        break;
                    }
                default:
                    panic("Unknown type specifier: %c", *c);
            }
            cs = false;
        } else {
            if (*c == '%') {
                cs = true;
            } else {
                print_char(*c);
            }
        }
    }
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}