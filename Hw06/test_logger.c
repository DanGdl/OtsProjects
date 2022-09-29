#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>

#include "dgd_logger.h"

int main(int argc, char* argv[]) {
    char* path = NULL;
    if (argc == 2) {
        path = argv[1];
    } else {
        printf("Please add a path to file to parameters\n");
        return 0;
    }

    DgdLogger_t* logger = DgdLog_init(path);
    if (logger == NULL) {
        printf("Failed to create logger\n");
    }

    DGD_LOG_DEBUG(logger, "test log1");
    DGD_LOG_INFO(logger, "test log2");
    DGD_LOG_WARN(logger, "test log3");
    DGD_LOG_ERROR(logger, "test log4");
    DGD_LOG_DEBUG(logger, NULL);
    DGD_LOG_DEBUG(logger, "");
    DGD_LOG_FATAL(logger, "WAAAAAGHH!!!");

    DgdLog_clean(logger);
    logger = NULL;
    return 0;
}
