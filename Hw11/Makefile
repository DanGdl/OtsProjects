log_analyzer: thread_pool.c hash_map.c util.c log_analyzer.c
	gcc -Wall -Wextra -Wpedantic -std=c11 hash_map.c util.c thread_pool.c log_analyzer.c -o log_analyzer -lpthread

all: log_analyzer

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions log_analyzer

.PHONY: clean
