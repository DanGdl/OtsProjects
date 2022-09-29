#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>


// #define SIGNATURE_START_OF_CENTRAL_RECORD 0x02014b50
#define SIGNATURE_END_OF_CENTRAL_RECORD 0x06054b50
#define SIGNATURE_START_OF_RECORD 0x04034b50

#define OFFSET_CENTRAL_ENTRY_SIZE 12    // offset from SIGNATURE_END_OF_CENTERAL_RECORD
#define OFFSET_ENTRY_NAME_SIZE 26       // offset from SIGNATURE_START_OF_RECORD
#define OFFSET_ENTRY_NAME 30            // offset from SIGNATURE_START_OF_RECORD


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
        uint16_t current_entry = 0;
        for (long int i = stats.st_size - 1; i >= 0; i--) {
            uint32_t signature = 0;
            memcpy(&signature, mapped + i, sizeof(signature));
            if (signature == SIGNATURE_END_OF_CENTRAL_RECORD) {
                uint32_t central_dir_size = 0;
                memcpy(&central_dir_size, mapped + i + OFFSET_CENTRAL_ENTRY_SIZE, sizeof(central_dir_size));

                // jump to beginning of central entry
                i = i - central_dir_size + 1;
            } else if (signature == SIGNATURE_START_OF_RECORD) {
                uint16_t name_size = 0;
                memcpy(&name_size, mapped + i + OFFSET_ENTRY_NAME_SIZE, sizeof(name_size));

                char name[name_size + 1];
                memcpy(&name, mapped + i + OFFSET_ENTRY_NAME, sizeof(name));
                name[name_size] = '\0';
                printf("Record %04d name %s\n", current_entry, (char*) &name);
                current_entry++;
            }
        }

        if (munmap(mapped, stats.st_size) != 0) {
            printf("Could not munmap\n");
        }
        close(fd);

        if (current_entry == 0) {
            printf("File doesn't contains zip archive\n");
        }
    }
    return 0;
}
