test_dgd_logger: lib_dgd_logger test_logger.c
	gcc -Wall -Wextra -Wpedantic -std=c11 -o test_dgd_logger test_logger.c -L. dgd_logger.a

lib_dgd_logger: dgd_logger.o
	ar cr dgd_logger.a dgd_logger.o

dgd_logger: dgd_logger.c
	gcc -rdynamic -Wall -Wextra -Wpedantic -std=c11 dgd_logger.c -o dgd_logger.o

all: test_dgd_logger

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions test_dgd_logger dgd_logger dgd_logger.a

.PHONY: clean
