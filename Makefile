NAME=image
MAKE=gmake

all: kernel image

kernel:
	@cd kern; $(MAKE)

image:
	@echo -e "[\033[0;34mGenerating image...\033[0;0m]"
	@dd if=/dev/zero of=boot/pad bs=1 count=750 2> /dev/null
	@cat boot/stage1 boot/stage2 boot/pad kern/kernel.bin > $(NAME).img
	@s1=`wc -c < boot/stage1`;\
	s2=`wc -c < boot/stage2`;\
	pad=`wc -c < boot/pad`;\
	kern=`wc -c < kern/kernel.bin`;\
	buf1=$$(( s1/512 + s2/512 + pad/512 + 1 ));\
	buf2=$$(( kern/512 + 1 ));\
	echo -e "To boot:\n\tqemu -fda $(NAME).img";\
	echo -e "grub: \tkernel $$buf1+$$buf2";\
	echo -e "grub: \tboot"
	@echo -e "[\033[0;34mdone\033[0;0m]";

test:
	qemu -fda $(NAME).img

clean:
	-rm *.img
	@cd kern; $(MAKE) clean

.PHONY: all