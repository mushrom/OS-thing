NAME=image
ARCH=i586
TARGET=$(ARCH)-elf
MAKE=gmake
EMULATOR=qemu
EMU_FLAGS=-hda vdrive.hdd -hdb $(NAME).hdd -s 
CROSS=$(shell pwd)/cross

KNAME=obsidian-$(ARCH)

CC=$(CROSS)/bin/$(TARGET)-gcc
LD=$(CROSS)/bin/$(TARGET)-ld
OBJCOPY=$(CROSS)/bin/$(TARGET)-objcopy
STRIP=$(CROSS)/bin/$(TARGET)-strip
#AS=$(CROSS)/bin/$(TARGET)-as
AS=nasm
#OBJCOPY=objcopy
#STRIP=strip
#CC=gcc
#LD=ld 
CONFIG_C_FLAGS=-g
#CONFIG_C_FLAGS=-g -DNO_DEBUG

all: check kernel image

debug:
	echo $(CROSS_PREFIX)

check:
	@if [ ! -e cross/.check-$(ARCH)-elf ]; then \
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
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)

image:
	@echo -e "[\033[0;34mGenerating image...\033[0;0m]"
	@dd if=/dev/zero of=boot/pad bs=1 count=750 2> /dev/null
	@kern=`wc -c < kern/$(KNAME).bin`; pad_s=$$(( 512 - kern%512 ));\
		dd if=/dev/zero of=boot/pad2 bs=1 count=$$pad_s 2> /dev/null
	@cat boot/stage1 boot/stage2 boot/pad kern/$(KNAME).bin boot/pad2 > $(NAME).hdd
	@sh mk_image.sh
	@s1=`wc -c < boot/stage1`;\
		s2=`wc -c < boot/stage2`;\
		pad=`wc -c < boot/pad`;\
		kern=`wc -c < kern/$(KNAME).bin`;\
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
	@cd cross; $(MAKE) MAKE=$(MAKE) TARGET=$(TARGET)
	@echo -e "[\033[0;34mdone\033[0;0m]"

clean:
	@cd kern; $(MAKE) clean

.PHONY: all
