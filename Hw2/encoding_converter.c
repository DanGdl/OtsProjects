#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>
#include <ctype.h>		// tolower


#include "cp1251.h"
#include "koi8-r.h"
#include "iso8859_5.h"

#define SIZE_BUFFER_ENCODING 100
#define SIZE_BUFFER 512
#define COUNT_ENCODINGS 3


typedef struct Converter {
    const char* const name;
    uint16_t (*converter)(uint8_t);
} Converter_t;


uint16_t cp1251_to_unicode(uint8_t symbol);
uint16_t iso8859_5_to_unicode(uint8_t symbol);
uint16_t koi8_to_unicode(uint8_t symbol);

const Converter_t encodings[COUNT_ENCODINGS] = {
    {
        .name = "cp1251",
        .converter = cp1251_to_unicode
    },
    {
        .name = "iso-8859-5",
        .converter = iso8859_5_to_unicode
    },
    {
        .name = "koi8-r",
        .converter = koi8_to_unicode
    }
};

int main(int argc, char* argv[]) {
    char* input_path = NULL;
	char* output_path = NULL;
	char encoding[SIZE_BUFFER_ENCODING] = { '\0' };
    if (argc == 4) {
        input_path = argv[1];
		output_path = argv[3];
		char* source = argv[2];
		char* result = encoding;
        const char* const end = encoding + SIZE_BUFFER_ENCODING - 1;
		while(*source != '\0' && result != end) {
			*result = tolower(*source);
			source++;
			result++;
		}
    } else {
        printf("Please add a path to input file to parameters, encoding and path to output file: encoding_converter input_file_path encoding outpu_file_path.\n");
        return 0;
    }
    const Converter_t* converter = NULL;
    for (int i = 0; i < COUNT_ENCODINGS; i++) {
        if (strcmp(encoding, encodings[i].name) == 0) {
            converter = &encodings[i];
            break;
        }
    }

    if (converter == NULL) {
        printf("Encoding %s is not supported. Supported encodings:", encoding);
        for (int i = 0; i < COUNT_ENCODINGS; i++) {
            if (i == COUNT_ENCODINGS - 1) {
                printf(" %s.", encodings[i].name);
            } else {
                printf(" %s,", encodings[i].name);
            }
        }
        printf("\n");
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
		FILE* output = fopen(output_path, "wb");
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
				uint8_t buffer[SIZE_BUFFER] = { 0 };

                // unicode encoding mark?!
				buffer[0] = 0xFF;
				buffer[1] = 0xFE;
				size_t idx = 2;

				for (long int i = 0; i < stats.st_size; i++) {
					uint8_t symbol = 0;
					memcpy(&symbol, mapped + i, sizeof(symbol));
					uint16_t s = (*converter -> converter)(symbol);
					size_t j = 0;
					while (j < sizeof(s)) {
						int shift = (j * 8);
						uint8_t tmpS = (s >> shift);
						buffer[idx] = tmpS;
						idx++;
						j++;
					}
					if (idx % SIZE_BUFFER == 0) {
						fwrite(buffer, sizeof(*buffer), SIZE_BUFFER, output);
						memset(buffer, 0, sizeof(buffer));
						idx = 0;
					}
				}
                fwrite(buffer, sizeof(*buffer), idx, output);
				fflush(output);

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


uint16_t cp1251_to_unicode(uint8_t symbol) {
	return cp_1252_symbols[symbol];
}


uint16_t iso8859_5_to_unicode(uint8_t symbol) {
	return iso8859_5_symbols[symbol];
}

uint16_t koi8_to_unicode(uint8_t symbol) {
	return koi8_symbols[symbol];
}
