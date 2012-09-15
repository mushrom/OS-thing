KNAME	   = kernel
ARCH	   = ia32
AS         = nasm
CC         = gcc
LD         = ld
CFLAGS     = -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -m32 \
		-Wall -DARCH=$(ARCH) -I$(shell pwd) -I$(shell pwd)/lib
LDFLAGS    = -T linker.ld -melf_i386_fbsd
LINKFILES  = $(KNAME).bin 

KFILES	  += $(shell find sys lib -name "*.c" ) 
KFILES    += $(shell find arch/$(ARCH)/ -name "*.s" )
KFILES	  += $(shell ls *.c *.s )
LINKFILES += $(shell echo $(KFILES) | sed "s/\.[c,h,s]/.o/g")

all: image

image: kernel link

debug:
	echo $(KFILES)
	echo $(LINKFILES)

kernel:
	for thing in $(KFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]/.o/"`; \
		EXT=`echo $$thing | tail -c3`; \
		echo "[ ] $$thing -> $$OUTFILE"; \
		if [ $$EXT = .c -o $$EXT = .h ]; then \
			$(CC) $(CFLAGS) -c $$thing -o $$OUTFILE; \
		elif [ $$EXT = .s ]; then \
			$(AS) -f elf $$thing; \
		fi; \
	done

link:
	$(LD) $(LDFLAGS) -o $(LINKFILES)

clean:
	rm $(LINKFILES)

.PHONY: all 
