
#ifndef H_DGD_LOGGER
#define H_DGD_LOGGER

#define ERR_TIME_FAIL       -1


#define INFO                "[INFO_]: "
#define DEBUG               "[DEBUG]: "
#define WARN                "[WARN_]: "
#define ERROR               "[ERROR]: "
#define FATAL               "[FATAL]: "

typedef struct DgdLogger DgdLogger_t;

DgdLogger_t* DgdLog_init(char* log_file_path);

void DgdLog_clean(DgdLogger_t* logger);


#define DGD_LOG_INFO(logger, message) write_log(logger, INFO, message, __FILE__, __LINE__, __func__)
#define DGD_LOG_DEBUG(logger, message) write_log(logger, DEBUG, message, __FILE__, __LINE__, __func__)
#define DGD_LOG_WARN(logger, message) write_log(logger, WARN, message, __FILE__, __LINE__, __func__)
#define DGD_LOG_ERROR(logger, message) write_log(logger, ERROR, message, __FILE__, __LINE__, __func__)
#define DGD_LOG_FATAL(logger, message) write_fatal(logger, message, __FILE__, __LINE__, __func__)

size_t write_log(DgdLogger_t* logger, const char* const tag, const char* const message, const char* const filename, int line, const char* const func_name);

size_t write_fatal(DgdLogger_t* logger, const char* const message, const char* const filename, int line, const char* const func_name);

#endif
