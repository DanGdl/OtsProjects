 dgd_hash_map: hash_map.c main.c util.c
	gcc -g -Wall -Wextra -Wpedantic -std=c11 hash_map.c main.c util.c -o dgd_hash_map

all: dgd_hash_map

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions dgd_plugin

.PHONY: clean
