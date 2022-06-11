#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>


#define SIGNATURE_END_OF_CENTERAL_RECORD 0x06054b50
#define OFFSET_TOTAL_ENTRIES 10 // offset from SIGNATURE_END_OF_CENTERAL_RECORD



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
        uint16_t total_entires = 0;
        for (int i = stats.st_size - 1; i >= 0; i--) {
            uint32_t signature = 0;
            memcpy(&signature, mapped + i, sizeof(signature));
            if (signature == SIGNATURE_END_OF_CENTERAL_RECORD) {
                memcpy(&total_entires, mapped + i + OFFSET_TOTAL_ENTRIES, sizeof(total_entires));
                break;
            }
        }

        if (munmap(mapped, stats.st_size) != 0) {
            printf("Could not munmap\n");
        }
        close(fd);

        if (total_entires == 0) {
            printf("File doesn't contains zip archive\n");
        } else {
            printf("File contains zip archive, total entries count %d\n", total_entires);
        }
    }
    return 0;
}
