ots_daemon: ots_daemon.c
	gcc -g -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L ots_daemon.c -o ots_daemon

ots_daemon_client: client.c
	gcc -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURC=200809L client.c -o client

all: ots_daemon_client ots_daemon

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions ots_daemon client

.PHONY: clean
