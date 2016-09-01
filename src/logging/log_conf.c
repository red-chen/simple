#include "log_conf.h"

#include "assert.h"
#include "string_tool.h"
#include "collection/array_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static enum LoggerLevel str_to_level(const char* str) {
    switch (*str) {
        case 'D':
            return LOG_DEBUG;
        case 'I':
            return LOG_INFO;
        case 'W':
            return LOG_WARN;
        case 'F':
            return LOG_FATAL;
        default:
            ASSERT(false, "Unsupport the log level :%s.", str);
            return LOG_DEBUG; // no use, just for fix compile warning
    }
}

static const char* level_to_str(enum LoggerLevel level) {
    switch (level) {
        case LOG_DEBUG: return LOG_LEVEL_DEBUG_STR;
        case LOG_INFO: return LOG_LEVEL_INFO_STR;
        case LOG_WARN: return LOG_LEVEL_WARN_STR;
        case LOG_ERROR: return LOG_LEVEL_ERROR_STR;
        case LOG_FATAL: return LOG_LEVEL_FATAL_STR;
        default:
            ASSERT(false, "Unsupport the log level :%d.", level);
            return LOG_LEVEL_DEBUG_STR; // no use, just for fix compile warning
    }
}

static bool str_to_bool(const char* str) {
    if (strcasecmp("true", str) == 0) {
        return true;
    } else {
        return false;
    }
}

static const char* bool_to_str(bool value) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

static char* check_and_get(Conf* conf, const char* key) {
    ASSERT_NOT_NULL(key);
    ASSERT_NOT_NULL(conf);

    char* value = conf_get(conf, key);
    ASSERT(value != NULL, "%s is missing", key);
    return value;
}

static char* try_and_get(Conf* conf, const char* key) {
    ASSERT_NOT_NULL(key);
    ASSERT_NOT_NULL(conf);

    return conf_get(conf, key);
}

static bool get_debug(Conf* conf, bool default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_GLOBAL_DEBUG);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return str_to_bool(tmp);
    }
}


ArrayList* get_logger_names(Conf* conf) {
    char* tmp = check_and_get(conf, LOG_GLOBAL_LOGGERS);
    char tmp_loggers_str[256] = { 0 };
    simple_string_trim(tmp, '\n', tmp_loggers_str);
    ArrayList* items = simple_string_split(tmp_loggers_str, ",");
    return items;
}

static void get_mapping(Conf* conf, const char* logger_name, char out[]) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_MAPPING, logger_name);
    char* tmp = check_and_get(conf, param_buf);
    strcpy(out, tmp);
}

static void get_path(Conf* conf, const char* logger_name, char out[]) {
    char param_buf[256] = { 0 };
    sprintf(param_buf, LOG_LOGGER_PATH, logger_name);
    char* tmp = check_and_get(conf, param_buf);
    strcpy(out, tmp);
}

static enum LoggerLevel get_level(Conf* conf, const char* logger_name) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_LEVEL, logger_name);
    char* tmp = check_and_get(conf, param_buf);
    enum LoggerLevel level = str_to_level(tmp);
    return level;
}

static bool get_stdout(Conf* conf, bool default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_GLOBAL_STDOUT);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return str_to_bool(tmp);
    }
}

static bool get_stdout_by_logger(Conf* conf, const char* logger_name, bool default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_STDOUT, logger_name);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return str_to_bool(tmp);
    }
}

static int get_filelimit(Conf* conf, int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_GLOBAL_FILE_LIMIT);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static int get_filelimit_by_logger(Conf* conf, const char* logger_name, int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_FILE_LIMIT, logger_name);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static int get_filesize(Conf* conf, int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_GLOBAL_FILE_SIZE_IN_BYTE);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static int get_filesize_by_logger(Conf* conf,const char* logger_name,  int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_FILE_SIZE_IN_BYTE, logger_name);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static int get_buffersize(Conf* conf, int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_GLOBAL_BUFFER_SIZE_IN_BYTE);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static int get_buffersize_by_logger(Conf* conf, const char* logger_name,  int default_value) {
    char param_buf[64] = { 0 };
    sprintf(param_buf, LOG_LOGGER_BUFFER_SIZE_IN_BYTE, logger_name);
    char* tmp = try_and_get(conf, param_buf);
    if (tmp == NULL) {
        return default_value;
    } else {
        return atoi(tmp);
    }
}

static void print_loggerconf(const char* logger_name, LoggerConf* logger) {
    fprintf(stderr, "{\n");
    fprintf(stderr, "\tlogger: %s,\n", logger_name);
    fprintf(stderr, "\tfile_limit: %d,\n", logger->file_limit);
    fprintf(stderr, "\tfile_size_in_byte: %d,\n", logger->file_size_in_byte);
    fprintf(stderr, "\tbuffer_size: %d,\n", logger->buffer_size);
    fprintf(stderr, "\tmapping: %s,\n", logger->mapping);
    fprintf(stderr, "\tpath: %s,\n", logger->path);
    fprintf(stderr, "\tlevel: %s,\n", level_to_str(logger->level));
    fprintf(stderr, "\tstdout: %s,\n", bool_to_str(logger->stdout));
    fprintf(stderr, "}\n");
}

LoggerConf* logconf_get_logger(Conf* conf, LoggingConf* lc, const char* logger_name) {
    LoggerConf* logger = malloc(sizeof(LoggerConf));
    ASSERT_NOT_NULL(logger);
    bzero(logger, sizeof(LoggerConf));

    logger->file_limit        = lc->file_limit;
    logger->file_size_in_byte = lc->file_size_in_byte;
    logger->buffer_size       = lc->buffer_size;

    // 必选
    get_mapping(conf, logger_name, logger->mapping);
    get_path(conf, logger_name, logger->path);
    logger->level  = get_level(conf, logger_name);

    // 可选
    logger->stdout            = get_stdout_by_logger(conf, logger_name, lc->stdout);
    logger->file_limit        = get_filelimit_by_logger(conf, logger_name, lc->file_limit);
    logger->file_size_in_byte = get_filesize_by_logger(conf, logger_name, lc->file_size_in_byte);
    logger->buffer_size       = get_buffersize_by_logger(conf, logger_name, lc->buffer_size);
    if (lc->debug) {
        print_loggerconf(logger_name, logger);
    }
    return logger;
}

LoggingConf* logconf_load(Conf* conf) {
    ASSERT_NOT_NULL(conf);
    ArrayList* loggers = arraylist_new(-1);
    // global
    LoggingConf* lc = malloc(sizeof(LoggingConf));
    lc->file_limit        = get_filelimit(conf, LOG_FILE_NUM_LIMIT);
    lc->file_size_in_byte = get_filesize(conf, LOG_FILE_SIZE_IN_BYTE);
    lc->buffer_size       = get_buffersize(conf, LOG_FILE_BUFFER_SIZE_IN_BYTE);
    lc->stdout            = get_stdout(conf, LOG_FILE_STDOUT);
    lc->debug             = get_debug(conf, LOG_FILE_DEBUG);
    lc->loggers           = loggers;

    ArrayList* names = get_logger_names(conf);

    // logger
    for (int i = 0; i < arraylist_size(names); i++) {
        const char* name = arraylist_get(names, i);
        ASSERT(strlen(name) != 0, "logger name is empty.");
        arraylist_add(loggers, logconf_get_logger(conf, lc, name));
    }
    return lc;
}
