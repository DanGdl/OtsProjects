async_client: async_client.c
	gcc -g -Wall -Wextra -Wpedantic -std=c11 async_client.c -o async_client

all: async_client

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions async_client

.PHONY: clean
