extern void inf_loop();

void kernel_entry() {
    *((short int*) 0xB8000 ) = (7 << 8) | 0x44;
    inf_loop();
}
