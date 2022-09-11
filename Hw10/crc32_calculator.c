#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>


typedef uint32_t crc;

#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0xD8  /* 11011 followed by 0's */


crc calculate_crc(crc remainder, uint8_t* const message, const int nBytes) {
	// Perform modulo-2 division, a byte at a time.
	for (int byte = 0; byte < nBytes; ++byte) {
		// Bring the next byte into the remainder.
		remainder ^= (message[byte] << (WIDTH - 8));

		 // Perform modulo-2 division, a bit at a time.
		for (uint8_t bit = 8; bit > 0; --bit) {
			// Try to divide the current data bit.
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			} else {
				remainder = (remainder << 1);
			}
		}
	}

	/*
	 * The final remainder is the CRC result.
	 */
	return remainder;
}

int main(int argc, char* argv[]) {
    char* path = NULL;
    if (argc == 2) {
        path = argv[1];
    } else {
        printf("Please add a path to file to parameters\n");
        return 0;
    }

    struct stat stats;
    if (stat(path, &stats) != 0) {
        printf("Can't get state for path %s\n", path);
    } else if (S_ISDIR(stats.st_mode)) {
        printf("File is directory, path %s\n", path);
    } else if (!S_ISREG(stats.st_mode)) {
        printf("File is not regular file, path %s\n", path);
    } else {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open file %s\n", path);
            return 0;
        }
        uint8_t* mapped = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf("Failed to map file to memory\n");
            close(fd);
            return 0;
        }
        crc remainder = 0;
		for (long int i = 0; i < stats.st_size; ++i) {
			remainder = calculate_crc(remainder, mapped + i, sizeof(*mapped));
		}

        printf("CRC32 remainder  for file %s: %d\n", path, remainder);

        if (munmap(mapped, stats.st_size) != 0) {
            printf("Could not munmap\n");
        }
        close(fd);
        fd = -1;
    }
    return 0;
}
