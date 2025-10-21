# =============================================================================
# Variables

# Build tools
NASM = nasm -felf
GCC_TEST = gcc -std=c99 -m32 -O2 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector
GCC_DEBUG = gcc -std=c99 -m32 -O0 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -DDEBUG
LD = ld -m elf_i386 -T link.ld -s


SRC = $(wildcard src/*.c)
C_OBJ = $(patsubst src/%.c,.tmp/%.o,$(SRC))

# =============================================================================
# Tasks

all: clean build test

.tmp/boot.o: src/boot.asm
	$(NASM) src/boot.asm -o .tmp/boot.o -dN=0xA000

.tmp/%.o: src/%.c
	$(GCC_DEBUG) -c $< -o $@

.tmp/os.elf: .tmp/boot.o $(C_OBJ) link.ld
	$(LD) .tmp/boot.o $(C_OBJ) -o .tmp/os.elf

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
