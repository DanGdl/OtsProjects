#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>


#define SIZE_BUFFER 512


int main(int argc, char* argv[]) {
    char* input_path = NULL;
	char* output_path = NULL;
	char* encoding = NULL;
    if (argc == 4) {
        input_path = argv[1];
		encoding = argv[2];
		input_path = argv[3];
    } else if () {
        printf("Please add a path to input file to parameters, encoding and path to output file: encoding_converter input_file_path encoding outpu_file_path\n");
        return 0;
    }

    struct stat stats;
    if (stat(input_path, &stats) != 0) {
        printf("Can't get state for path %s\n", input_path);
    } else if (S_ISDIR(stats.st_mode)) {
        printf("File is directory, path %s\n", input_path);
    } else if (!S_ISREG(stats.st_mode)) {
        printf("File is not regular file, path %s\n", input_path);
    } else {
		FILE* output = fopen(output_path, "rwb");
		if (output == NULL) {
			printf("Failed to open file %s\n", output_path);
			return 0;
		}
        const int fd = open(input_path, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open file %s\n", input_path);
        } else {
			uint8_t* const mapped = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapped == MAP_FAILED) {
				printf("Failed to map file to memory\n");
			} else {
				// TODO: select proper type
				uint8_t* const buffer[SIZE_BUFFER] = { 0 };
				uint8_t symbol;
				for (long int i = 0; i < stats.st_size; i++) {
					memcpy(&symbol, mapped + i, sizeof(symbol));
					// TODO: convert encoding
					if (i % SIZE_BUFFER == 0) {
						fwrite(output, buffer);
						memset(&buffer, 0, sizeof(buffer));
					}
				}
				fwrite(output, buffer);
				
				if (munmap(mapped, stats.st_size) != 0) {
					printf("Could not munmap\n");
				}
			}
			close(fd);
		}
		fclose(output);
    }
    return 0;
}
