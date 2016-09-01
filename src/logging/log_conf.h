#ifndef LOG_CONF_H_
#define LOG_CONF_H_

#include "log_file_manager.h"

#include "logging.h"
#include "conf.h"

#include "collection/array_list.h"

#include <stdbool.h>

// Level string

#define LOG_LEVEL_DEBUG_STR "DEBUG"
#define LOG_LEVEL_INFO_STR  "INFO"
#define LOG_LEVEL_WARN_STR  "WARN"
#define LOG_LEVEL_ERROR_STR "ERROR"
#define LOG_LEVEL_FATAL_STR "FATAL"

// Configure parameter

#define LOG_GLOBAL_DEBUG               "global.debug"

#define LOG_GLOBAL_LOGGERS             "global.loggers"
#define LOG_GLOBAL_STDOUT              "global.stdout"
#define LOG_GLOBAL_FILE_LIMIT          "global.filelimit"
#define LOG_GLOBAL_FILE_SIZE_IN_BYTE   "global.filesize_in_byte"
#define LOG_GLOBAL_BUFFER_SIZE_IN_BYTE "global.buffersize_in_byte"

#define LOG_LOGGER_MAPPING             "logger.%s.mapping"
#define LOG_LOGGER_LEVEL               "logger.%s.level"
#define LOG_LOGGER_PATH                "logger.%s.path"
#define LOG_LOGGER_STDOUT              "logger.%s.stdout"
#define LOG_LOGGER_FILE_LIMIT          "logger.%s.filelimit"
#define LOG_LOGGER_FILE_SIZE_IN_BYTE   "logger.%s.filesize_in_byte"
#define LOG_LOGGER_BUFFER_SIZE_IN_BYTE "logger.%s.buffersize_in_byte"

// Default value
#define LOG_FILE_NUM_LIMIT           (10)
#define LOG_FILE_SIZE_IN_BYTE        (2 * 1024 * 1024)
#define LOG_FILE_BUFFER_SIZE_IN_BYTE (64 * 1024 )
#define LOG_FILE_STDOUT              false
#define LOG_FILE_DEBUG               false

typedef struct loggerconf_t {
    char mapping [128];
    char path [256]; // 日志文件输出路径
    enum LoggerLevel level; // 日志打印级别
    int file_limit; // 文件个数的限制
    int file_size_in_byte; // 文件大小的限制
    enum LogFileManagerMode mode;
    int buffer_size; // 缓存的大小
    bool stdout; // 是否在控制台输出
} LoggerConf;

typedef struct loggingconf_t {
    int file_limit;
    int file_size_in_byte;
    int buffer_size;
    bool stdout;
    bool debug; // 是否开启调试模式
    ArrayList* loggers;
} LoggingConf;

LoggingConf* logconf_load(Conf* conf);

#endif /* LOG_CONF_H_ */
