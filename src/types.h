#pragma once

typedef char i8;
typedef unsigned char u8;

typedef short i16;
typedef unsigned short u16;

typedef int i32;
typedef unsigned int u32;

typedef long long i64;
typedef unsigned long long u64;

extern void inf_loop();
extern void cli();
extern void sti();
extern void div_zero();
extern void pseudo_syscall();
extern void lidt(u64* idt_address);
extern void setup_reg();
extern u32 eflags();
extern void restore_context(void* context);
extern void syscall_expr(int n);
extern u32 get_esp();
extern void not_allowed();
extern u64 code_segment_desc;

#define NULL ((void*) 0)

#define true 1
#define false 0