KNAME	   = kernel
MAKE	   = gmake
ARCH	   = i586
AS         = nasm
CC         = gcc
LD         = ld
OBJCOPY    = objcopy
STRIP	   = strip
CFLAGS     = -nostartfiles -nodefaultlibs -nostdlib -fno-builtin \
		-Wall -DARCH=$(ARCH) -I$(shell pwd)/include
CONFIG_C_FLAGS = 
CFLAGS 	  += $(CONFIG_C_FLAGS)
LDFLAGS    = -T linker.ld
LINKFILES  = $(KNAME).bin 

KFILES	  += $(shell find sys kern lib fs mem drivers -name "*.[c,s]" ) 
KFILES	  += $(shell ls *.c *.s )
LINKFILES += $(shell echo $(KFILES) | sed "s/\.[c,h,s]/.o/g")

MODFILES   = $(shell find modules/mods -name "*.[c]")

all: kernel link extract-syms kmodules

debug:
	@echo "Kfiles: $(KFILES)"
	@echo "Linkfiles: $(LINKFILES)"

kernel:
	@echo -e "[\033[0;34mCompiling kernel...\033[0;0m]"
	@for thing in $(KFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]/.o/"`; \
		EXT=`echo $$thing | tail -c3`; \
		RET=0; \
		echo -e "Compiling $$thing...\r\c"; \
		if [ $$EXT = .c ]; then \
			$(CC) $(CFLAGS) -c $$thing -o $$OUTFILE; \
			RET=$$?; \
		elif [ $$EXT = .s ]; then \
			$(AS) -f elf $$thing; \
			RET=$$?; \
		fi; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE"; \
		fi; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"

link:
	@echo -e "[\033[0;34mLinking...\033[0;0m]"
	@$(LD) $(LDFLAGS) -o $(LINKFILES)
	@echo "Kernel name: $(KNAME).bin"
	@echo -e "[\033[0;34mdone\033[0;0m]"

extract-syms:
	@echo -e "[\033[0;34mExtracting debug symbols...\033[0;0m]"
	@$(OBJCOPY) --only-keep-debug $(KNAME).bin $(KNAME).sym
	@$(STRIP) --strip-debug --strip-unneeded $(KNAME).bin
	@echo "symbol file: $(KNAME).sym"
	@echo -e "[\033[0;34mdone\033[0;0m]"

kmodules:
	@echo -e "[\033[0;34mCompiling modules...\033[0;0m]"
	@$(AS) -felf modules/lib/mrt0.s
	@$(CC) $(CFLAGS) -c modules/lib/syscall.c -o modules/lib/syscall.o
	@for thing in $(MODFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]//"`; \
		EXT=`echo $$thing | tail -c3`; \
		RET=0; \
		echo -e "Compiling $$thing...\r\c"; \
		if [ $$EXT = .c -a $$OUTFILE != syscall ]; then \
			$(CC) $(CFLAGS) -c $$thing -o $$OUTFILE.o; \
			RET=$$?; \
		fi; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			$(LD) -o $$OUTFILE.ko modules/lib/mrt0.o $$OUTFILE.o modules/lib/syscall.o; \
			cp $$OUTFILE.ko ../init; \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE.ko"; \
		fi; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"
	#$(LD) -o $$OUTFILE.ko modules/mrt0.o $$OUTFILE.o obsidian-i586.sym; \

test:
	qemu -kernel $(KNAME).bin

clean:
	-rm $(LINKFILES)
	-rm *.sym *.bin

.PHONY: all 
