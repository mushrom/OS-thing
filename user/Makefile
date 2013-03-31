ASFILES=$(wildcard ./*.s)
LIBFILES=syscall.c stdio.c
CFILES=$(wildcard ./*.c)
LIBFILES=$(wildcard ./lib/*.c)
LFILES=$(patsubst %.c,%.o,$(LIBFILES))
LD=../cross/bin/i586-elf-ld
CC=../cross/bin/i586-elf-gcc
CFLAGS=-nodefaultlibs -nostartfiles -fno-builtin -I. -I./include

all: libs userbin link copy

libs:
	@echo -e "[\033[0;34mCompiling user libs...\033[0;0m]"
	@for thing in $(LIBFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]/.o/"`; \
		echo -e "Compiling $$thing...\r\c"; \
		$(CC) -c $$thing $(CFLAGS) -o $$OUTFILE; \
		RET=$$?; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE"; \
		fi; \
	done
	@nasm -f elf lib/crt0.s
	@echo -e "[\033[0;34mdone\033[0;0m]"

userbin:
	@echo -e "[\033[0;34mMaking userland...\033[0;0m]"
	@for thing in $(CFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]/.o/"`; \
		echo -e "Compiling $$thing...\r\c"; \
		$(CC) $(CFLAGS) -c $$thing; \
		RET=$$?; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE"; \
		fi; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"

link:
	@echo -e "[\033[0;34mLinking userland...\033[0;0m]"
	@mkdir -p bin
	@for thing in $(CFILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]//"`; \
		OBFILE=`echo $$thing | sed "s/\.[c,h,s]/.o/"`; \
		echo -e "Linking $$thing...\r\c"; \
		$(LD) -o $$OUTFILE lib/crt0.o $$OBFILE $(LFILES); \
		RET=$$?; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			mv $$OUTFILE ./bin; \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE"; \
		fi; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"

copy:
	@echo -e "[\033[0;34mMoving userland to initrd...\033[0;0m]"
	@for thing in `ls bin`; do \
		cp bin/$$thing ../init/; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"

clean: 
	rm bin/*
	rm *.o
	rm lib/*.o

.PHONY: all
