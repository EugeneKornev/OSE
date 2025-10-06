#pragma once

#include <stdarg.h>
#include "types.h"

void clear_screen();

void set_bg_color(u8 color);
void set_fg_color(u8 color);

void vprintf(const char* foramt, va_list args);
void printf(const char* foramt, ...);