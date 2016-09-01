// 主要管理日志文件
// 1.支持循环产生文件
// 2.支持按照天的级别产生文件
// 3.支持按照小时级别产生文件

#ifndef LOG_FILE_MANAGER_H_
#define LOG_FILE_MANAGER_H_

#define LOG_FILE_MANAGER_DEFAULT_ROLLING_FILE_COUNT (10)

enum LogFileManagerMode {
    LOG_FILE_MANAGER_ROLLING, // 文件轮训
    LOG_FILE_MANAGER_DAY,
    LOG_FILE_MANAGER_HOUR,
};

typedef struct logfilemanager_t LogFileManager;

LogFileManager* logfilemanager_new(
                const char* path,
                enum LogFileManagerMode mode,
                int file_size,
                int file_num,
                int buffer_size);

void logfilemanager_close(LogFileManager* self);

void logfilemanager_set_file_limit(LogFileManager* self, int limit);

void logfilemanager_write_line(LogFileManager* self, const char* line);

void logfilemanager_flush(LogFileManager* self);

#endif /* LOG_FILE_MANAGER_H_ */
