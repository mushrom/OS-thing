NAME=image
MAKE=gmake
EMULATOR=qemu
EMU_FLAGS=-hdb vdrive.img -hda
CC=$(shell pwd)/cross/bin/i586-elf-gcc
LD=$(shell pwd)/cross/bin/i586-elf-ld
#CC=gcc
#LD=ld
CONFIG_C_FLAGS=

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
	@cd kern; $(MAKE) CONFIG_C_FLAGS="$(CONFIG_C_FLAGS)" CC=$(CC) LD=$(LD)

image:
	@echo -e "[\033[0;34mGenerating image...\033[0;0m]"
	@dd if=/dev/zero of=boot/pad bs=1 count=750 2> /dev/null
	@cat boot/stage1 boot/stage2 boot/pad kern/kernel.bin > $(NAME).img
	@dd if=/dev/zero of=vdrive.img bs=1 count=8096 2> /dev/null
	@s1=`wc -c < boot/stage1`;\
		s2=`wc -c < boot/stage2`;\
		pad=`wc -c < boot/pad`;\
		kern=`wc -c < kern/kernel.bin`;\
		buf1=$$(( s1/512 + s2/512 + pad/512 + 1 ));\
		buf2=$$(( kern/512 ));\
		echo -e "To boot:\n\t$(EMULATOR) $(EMU_FLAGS) $(NAME).img";\
		echo -e "grub: \tkernel $$buf1+$$buf2";\
		echo -e "grub: \tboot"
	@echo -e "[\033[0;34mdone\033[0;0m]";

test:
	$(EMULATOR) $(EMU_FLAGS) $(NAME).img

cross-cc:
	@echo -e "[\033[0;34mMaking cross-compiler...\033[0;0m]"
	@cd cross; $(MAKE) MAKE=$(MAKE)
	@echo -e "[\033[0;34mdone\033[0;0m]"

clean:
	-rm *.img
	@cd kern; $(MAKE) clean

.PHONY: all
