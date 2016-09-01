// 异步储存数据

#ifndef LEMON_ASYNC_FILE_H_
#define LEMON_ASYNC_FILE_H_

typedef struct asyncfile_t AsyncFile;

// 当文件Buffer满了之后，系统的行为模式
enum AsyncMode
{
    ASYNC_MODE_BLOCK,     // 系统Block用户写入
    ASYNC_MODE_DISCARDED,  // 系统继续接收写入，但是将多的数据丢弃
};

enum WriteMode {
    CACHE, // 写到操作系统的cache就返回
    NO_CACHE, // 写透磁盘
};

AsyncFile* asyncfile_new(
        const char* path,
        enum AsyncMode async_mode,
        enum WriteMode write_mode,
        int buf_num,
        int buf_size);

void asyncfile_close(AsyncFile* self);

void asyncfile_write(AsyncFile* self, const void* data, int size);

void asyncfile_write_line(AsyncFile* self, const char* line);

void asyncfile_flush(AsyncFile* self);

#endif /* LOMON_ASYNC_FILE_H_ */
