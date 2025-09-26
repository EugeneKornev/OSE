# =============================================================================
# Variables

# Build tools
NASM = nasm -felf64


# Flags
GCC_FLAGS = -std=c99 -m i386 -O2 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector


# =============================================================================
# Tasks

all: clean build test

.tmp/boot.o: src/boot.asm
	$(NASM) src/boot.asm -o .tmp/boot.o -dN=0xA000

.tmp/main.o: src/main.c
	gcc $(GCC_FALGS) -c src/main.c -o .tmp/main.o

.tmp/os.elf: .tmp/boot.o .tmp/main.o link.ld
	ld -s .tmp/main.o .tmp/boot.o -T link.ld -o .tmp/os.elf

.tmp/os.bin: .tmp/os.elf
	objcopy -I elf32-i386 -O binary .tmp/os.elf .tmp/os.bin
	

boot.img: .tmp/os.bin
	dd if=/dev/zero of=boot.img bs=1024 count=1440
	dd if=.tmp/os.bin of=boot.img conv=notrunc

build: boot.img

clean:
	rm -f *.img
	rm -rf .tmp
	mkdir .tmp

test: build
	qemu-system-i386 -cpu pentium2 -m 1g -fda boot.img -monitor stdio -device VGA

debug: build
	qemu-system-i386 -cpu pentium2 -m 1g -fda boot.img -monitor stdio -device VGA -s -S &
	gdb -x .gdbinit

.PHONY: all build clean test debug
