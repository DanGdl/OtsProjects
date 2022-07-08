dgd_plugin: dgd_plugin.c
	gcc -Wall -Wextra -Wpedantic -std=c11 dgd_plugin.c -o dgd_plugin

all: dgd_plugin

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions dgd_plugin

.PHONY: clean
