NAME=image
KNAME=kernel
MAKE=gmake
EMULATOR=qemu
EMU_FLAGS=-hdb vdrive.img -hda $(NAME).img -gdb tcp::9000
CROSS=$(shell pwd)/cross

CC=$(CROSS)/bin/i586-elf-gcc
LD=$(CROSS)/bin/i586-elf-ld
OBJCOPY=$(CROSS)/bin/i586-elf-objcopy
STRIP=$(CROSS)/bin/i586-elf-strip
#OBJCOPY=objcopy
#STRIP=strip
#CC=gcc
#LD=ld 
#CONFIG_C_FLAGS=
CONFIG_C_FLAGS=-g

all: check kernel image

debug:
	echo $(CROSS_PREFIX)

check:
	@if [ ! -e cross/.cross_check ]; then \
		echo "-----=[Note]=-----";\
		echo "It is recommended that you build a cross compiler using"; \
		echo "    \"$(MAKE) cross-cc\""; \
		echo "if you haven't done so already, for best results."; \
		echo;\
		echo "Your native compiler may work, but if it refuses to boot or compile,";\
		echo "try a cross compiler before assuming bugs. ;)";\
		echo;\
	fi

kernel:
	@cd kern; $(MAKE) KNAME=$(KNAME) CONFIG_C_FLAGS="$(CONFIG_C_FLAGS)" \
			  CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY)

image:
	@echo -e "[\033[0;34mGenerating image...\033[0;0m]"
	@dd if=/dev/zero of=boot/pad bs=1 count=750 2> /dev/null
	@kern=`wc -c < kern/kernel.bin`; pad_s=$$(( 512 - kern%512 ));\
		dd if=/dev/zero of=boot/pad2 bs=1 count=$$pad_s 2> /dev/null
	@cat boot/stage1 boot/stage2 boot/pad kern/kernel.bin boot/pad2 > $(NAME).img
	@if [ ! -e vdrive.img ]; then \
		dd if=/dev/zero of=vdrive.img bs=1 count=8096 2> /dev/null; \
	fi
	@s1=`wc -c < boot/stage1`;\
		s2=`wc -c < boot/stage2`;\
		pad=`wc -c < boot/pad`;\
		kern=`wc -c < kern/kernel.bin`;\
		buf1=$$(( s1/512 + s2/512 + pad/512 + 1 ));\
		buf2=$$(( kern/512 + 1));\
		echo -e "To boot:\n\t$(EMULATOR) $(EMU_FLAGS)";\
		echo -e "grub: \tkernel $$buf1+$$buf2";\
		echo -e "grub: \tboot"
	@echo -e "[\033[0;34mdone\033[0;0m]";

test:
	$(EMULATOR) $(EMU_FLAGS)

cross-cc:
	@echo -e "[\033[0;34mMaking cross-compiler...\033[0;0m]"
	@cd cross; $(MAKE) MAKE=$(MAKE)
	@echo -e "[\033[0;34mdone\033[0;0m]"

clean:
	-rm *.img
	@cd kern; $(MAKE) clean

.PHONY: all
