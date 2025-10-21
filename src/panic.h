#pragma once

#include <stdarg.h>

void vpanic(const char* format, va_list args);
void panic(const char* format, ...);
