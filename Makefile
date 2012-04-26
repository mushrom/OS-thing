AS        = nasm
CC        = gcc
LD        = ld
CFLAGS    = -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -m32
LDFLAGS   = -T linker.ld -melf_i386
LINKFILES = kernel.bin kernel.o loader.o

all: Image

Image: Loader Kernel Linker

Kernel:
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

Loader:
	$(AS) -f elf loader.s

Linker:
	$(LD) $(LDFLAGS) -o $(LINKFILES)

clean:
	rm kernel.o kernel.bin loader.o
