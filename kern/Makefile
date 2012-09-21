KNAME	   = kernel
ARCH	   = ia32
AS         = nasm
CC         = gcc
LD         = ld
#CFLAGS     = -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -m32 \
		-Wall -DARCH=$(ARCH) -I$(shell pwd) -I$(shell pwd)/lib
CFLAGS     = -nostdlib -fno-builtin -m32 \
		-Wall -DARCH=$(ARCH) -I$(shell pwd) -I$(shell pwd)/lib
LDFLAGS    = -T linker.ld -melf_i386_fbsd
LINKFILES  = $(KNAME).bin 

KFILES	  += $(shell find sys lib arch/$(ARCH)/ -name "*.[c,s]" ) 
KFILES	  += $(shell ls *.c *.s )
LINKFILES += $(shell echo $(KFILES) | sed "s/\.[c,h,s]/.o/g")

all: kernel link

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

test:
	qemu -kernel $(KNAME).bin

clean:
	-rm $(LINKFILES)

.PHONY: all 
