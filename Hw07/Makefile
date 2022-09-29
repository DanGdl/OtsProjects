asm_check: asm_check.c
	gcc -g -Wall -Wextra -Wpedantic -std=c11 asm_check.c -o asm_check

all: asm_check

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions asm_check

.PHONY: clean
