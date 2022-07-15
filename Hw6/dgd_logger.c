
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <execinfo.h>


#include "dgd_logger.h"


#define DATE_BUFFER_SIZE    21 // 2022-07-15 10:40:20
#define BACK_TRACE_SIZE     30 // maybe enough


typedef struct DgdLogger {
    FILE* p_file;
} DgdLogger_t;


DgdLogger_t* DgdLog_init(char* log_file_path) {
    if (log_file_path == NULL || strlen(log_file_path) == 0) {
        return NULL;
    }
    struct stat stats;
    if (stat(log_file_path, &stats) == 0) {
        if (S_ISDIR(stats.st_mode)) {
            printf("File is directory, path %s\n", log_file_path);
            return NULL;
        } else if (!S_ISREG(stats.st_mode)) {
            printf("File is not regular file, path %s\n", log_file_path);
            return NULL;
        }
    }
    
    DgdLogger_t* logger = NULL;
    logger = calloc(sizeof(*logger), 1);
    if (logger == NULL) {
        return NULL;
    }
    logger -> p_file = fopen(log_file_path, "ar");
    if (logger -> p_file == NULL) {
        return NULL;
    }
    return logger;
}

void DgdLog_clean(DgdLogger_t* logger) {
    if (logger == NULL) {
        return;
    }
    if (logger -> p_file != NULL) {
        fclose(logger -> p_file);
        logger -> p_file = NULL;
    }
    free(logger);
}



size_t write_log(DgdLogger_t* logger, const char* const tag, const char* const message, const char* const filename, int line, const char* const func_name) {
    if (logger == NULL || logger -> p_file == NULL) {
        return ERR_BAD_LOGGER;
    }
    time_t now = time(NULL);
    if (now == -1) {
        return ERR_TIME_FAIL;
    }
    struct tm* ptm = localtime(&now);
    if (ptm == NULL) {
        return ERR_TIME_FAIL;
    }
    char buf[DATE_BUFFER_SIZE] = {0};
    // %F == %Y-%m-%d, %R == %HH:%MM:%S
    strftime(&buf[0], DATE_BUFFER_SIZE, "%F %T.", ptm);

    size_t consumed = 0;
    if (message == NULL || strlen(message) == 0) {
        consumed = fprintf(logger -> p_file, "%s%03ld %s %s:%d %s\n", &buf[0], now % 1000, tag, filename, line, func_name);
    } else {
        consumed = fprintf(logger -> p_file, "%s%03ld %s %s:%d %s %s\n", &buf[0], now % 1000, tag, filename, line, func_name, message);
    }
    return consumed;
}

size_t write_fatal(DgdLogger_t* logger, const char* const message, const char* const filename, int line, const char* const func_name) {
    if (logger == NULL || logger -> p_file == NULL) {
        return ERR_BAD_LOGGER;
    }

    size_t consumed = write_log(logger, FATAL, message, filename, line, func_name);;

    void* array[BACK_TRACE_SIZE];
    const int size = backtrace(array, BACK_TRACE_SIZE);
    char** strings = backtrace_symbols(array, size);
    if (strings != NULL) {
        for (int i = size - 1; i >= 0; i--) {
            consumed += fprintf(logger -> p_file, "%s\n", strings[i]);
        }
    }
    free(strings);
    return consumed;
}
