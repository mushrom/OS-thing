FILES=*.s
LD=../cross/bin/i586-elf-ld

all: bin

bin:
	@echo -e "[\033[0;34mMaking userland...\033[0;0m]"
	@for thing in $(FILES); do \
		OUTFILE=`echo $$thing | sed "s/\.[c,h,s]//"`; \
		echo -e "Assembling $$thing...\r\c"; \
		nasm -f elf $$thing; \
		RET=$$?; \
		if [ $$RET -gt 0 ]; then \
			echo -e "[\033[0;31merror\033[0;0m] $$thing -> $$OUTFILE"; \
		else \
			echo -e "[\033[0;32mdone\033[0;0m] $$thing -> $$OUTFILE"; \
			$(LD) -o $$OUTFILE $$OUTFILE.o; \
			cp $$OUTFILE ../init/$$OUTFILE; \
		fi; \
	done
	@echo -e "[\033[0;34mdone\033[0;0m]"

.PHONY: all
