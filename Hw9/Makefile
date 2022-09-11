ots_daemon: ots_daemon.c
	gcc -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE ots_daemon.c -o ots_daemon

ots_daemon_client: client.c
	gcc -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE client.c -o client

all: ots_daemon_client, ots_daemon

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions ots_daemon client

.PHONY: clean
