#pragma once
#include "types.h"

struct interrupt_context;

void* setup_interrupts();

void universal_handler(struct interrupt_context* context);

void setup_Intel8259(bool autoEOI);

void set_master_mask(u8 mask);

extern int global_counter;
