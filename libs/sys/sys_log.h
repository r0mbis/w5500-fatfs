#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_DEFAULT 5

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEFAULT
#endif

#define LOG_CLR(clr) "\033[0;" clr "m"
#define LOG_CLR_RESET "\033[0m"

#define LOG_CLR_BLACK "30"
#define LOG_CLR_RED "31"
#define LOG_CLR_GREEN "32"
#define LOG_CLR_BROWN "33"
#define LOG_CLR_BLUE "34"
#define LOG_CLR_PURPLE "35"

#define LOG_CLR_E LOG_CLR(LOG_CLR_RED)
#define LOG_CLR_W LOG_CLR(LOG_CLR_BROWN)
#define LOG_CLR_I LOG_CLR(LOG_CLR_GREEN)
#define LOG_CLR_D LOG_CLR(LOG_CLR_BLUE)
#define LOG_CLR_V LOG_CLR(LOG_CLR_PURPLE)

typedef void (*SysLogStdout)(void*, char);
typedef int16_t (*SysLogStdin)(void);
typedef uint32_t (*SysLogTimestamp)(void);
typedef void (*SysLogCustomOut)( char* log_string);  // New Protobuf output function

typedef enum {
    LOG_NONE = 0,
    LOG_INFO = 1,
    LOG_ERROR = 2,
    LOG_WARN = 3,
    LOG_DEBUG = 4,
    LOG_VERBOSE = 5,
    LOG_TOTAL = 6,
} SysLogLevel;

typedef struct {
    SysLogLevel log_level;
    SysLogStdout out;
    SysLogStdin in;
    SysLogCustomOut custom_out;
    SysLogTimestamp timetamp;
} SysLogParams;

void sys_log_init();
void sys_log_set_level(SysLogLevel level);
void sys_log_set_uart();
void sys_log_set_custom(SysLogCustomOut out);
void sys_log_set_swo();
void stdout_putf(char c);
int16_t stdout_getc();
void sys_log_print(SysLogLevel level, const char* tag, const char* format, ...);
void sys_log_print_str(const char* str);

#define APPEND_NEWLINE(format) format "\r\n"

#define log_e(tag, format, ...) sys_log_print(LOG_ERROR, tag, APPEND_NEWLINE(format), ##__VA_ARGS__)
#define log_w(tag, format, ...) sys_log_print(LOG_WARN, tag, APPEND_NEWLINE(format), ##__VA_ARGS__)
#define log_i(tag, format, ...) sys_log_print(LOG_INFO, tag, APPEND_NEWLINE(format), ##__VA_ARGS__)
#define log_d(tag, format, ...) sys_log_print(LOG_DEBUG, tag, APPEND_NEWLINE(format), ##__VA_ARGS__)
#define log_dn(tag, format, ...) sys_log_print(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define log_v(tag, format, ...) sys_log_print(LOG_VERBOSE, tag, APPEND_NEWLINE(format), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
