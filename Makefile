ARCH=i586
TARGET=$(ARCH)-elf
MAKE=gmake
EMULATOR=qemu
EMU_FLAGS=-hda vdrive.hdd -s 
CROSS=$(shell pwd)/cross

KNAME=obsidian-$(ARCH)

NATIVECC=gcc
CC=$(CROSS)/bin/$(TARGET)-gcc
LD=$(CROSS)/bin/$(TARGET)-ld
OBJCOPY=$(CROSS)/bin/$(TARGET)-objcopy
STRIP=$(CROSS)/bin/$(TARGET)-strip
AS=nasm
#CC=gcc
#LD=ld 
CONFIG_C_FLAGS=-g -DRECOVER_FROM_PANIC
#CONFIG_C_FLAGS=-g -DNO_DEBUG
#CONFIG_C_FLAGS=-g

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
	@cd tools; $(NATIVECC) -o mkinitrd mkinitrd.c
	@cd init; ../tools/mkinitrd ../initrd.img *
	@sh mk_image.sh
	@echo "To boot: $(EMULATOR) $(EMU_FLAGS)"
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
