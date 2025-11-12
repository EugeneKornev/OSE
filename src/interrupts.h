#pragma once

struct interrupt_context;

void setup_interrupts();

void universal_handler(struct interrupt_context* context);
