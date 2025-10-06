#pragma once

#include "types.h"

void* calloc_linear(u32 sise, u32 align);

void* malloc_linear(u32 size, u32 align);

void* memcpy(void* dst, const void* src, u32 size);

void* copy_forward(void* dst, const void* src, u32 size);

void* copy_backward(void* dst, const void* src, u32 size);

void* memmove(void* dst, const void* src, u32 size);

void* memzero(void* dst, u32 size);