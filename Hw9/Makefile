ots_daemon: ots_daemon.c
	gcc -Wall -Wextra -Wpedantic -std=c11 -D_XOPEN_SOURCE=500 ots_daemon.c -o ots_daemon

all: ots_daemon

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions ots_daemon

.PHONY: clean
