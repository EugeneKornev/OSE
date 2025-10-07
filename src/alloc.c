#include "alloc.h"
#include "panic.h"


static u8 arena_end = (u8) 0x400000;
static u8* current_address = (u8*) 0x100000;


void* malloc_linear(u32 size, u32 align) {
    if (size == 0) {
        return NULL;
    }

    if (align == 0) {
        align = 1;
    }

    if ((u32) current_address % align != 0) {
        current_address += (align - (u32) current_address % align);
    }

    if ((u32) current_address + size > arena_end) {
        panic("Failed to alloc %d byted. Last allocated address: %x", size, current_address);
    }

    void* result = current_address;
    current_address += size;
    return result;
}


void* calloc_linear(u32 size, u32 align) {
    void* result = malloc_linear(size, align);
    if (result == NULL) {
        return result;
    }
    memzero(result, size);
    return result;
}


void* memcpy(void* dst, const void* src, u32 size) {
    return copy_forward(dst, src, size);   
}

void* copy_forward(void* dst, const void* src, u32 size) {
    for (u32 i = 0; i < size; i++) {
        *((u8*)dst + i) = *((u8*) src + i); // cast to (u8*) to copy by bytes
    }
    return dst;
}

void* copy_backward(void* dst, const void* src, u32 size) {
    for (u32 i = size; i-- > 0; ) {
        *((u8*)dst + i) = *((u8*)src + i); 
    }
    return dst;
}

void* memmove(void* dst, const void* src, u32 size) {
    if ((u32) dst + size > (u32) src) {      // <----dst---->
        copy_forward(dst, src, size);  //          <----src---->
    } else {
        copy_backward(dst, src, size);
    }
    return dst;
}

void* memzero(void* dst, u32 size) {
    for (u32 i = 0; i < size; i++) {
        *((u8*)dst + i) = 0;
    }
    return dst;
}