#include "sys_log.h"
#include "printf.h"
#include "usart.h"
#include "bsp_vcp.h"

#define MESSAGE_MAX_LENGTH 4096
char log_message_buffer[MESSAGE_MAX_LENGTH];

static const char* log_colors[LOG_TOTAL] = {
    [LOG_INFO] = LOG_CLR_I,
    [LOG_ERROR] = LOG_CLR_E,
    [LOG_WARN] = LOG_CLR_W,
    [LOG_DEBUG] = LOG_CLR_D,
    [LOG_VERBOSE] = LOG_CLR_V,
};

static SysLogParams sys_log;

static void stdout_putf_uart(void* unused, char c) {
    UNUSED(unused);
    bsp_vcp_tx(&c);
}

static void stdout_putf_swo(void* unused, char c) {
    UNUSED(unused);
    ITM_SendChar(c);
}


void stdout_putf(char c) {
    sys_log.out(NULL, c);
}

void _putchar(char character) {
    sys_log.out(NULL, character);
}


static void _stdout_putf(char c, void* unused) {
    sys_log.out(NULL, c);
}

// setup stdout callback and init printf lib
static void sys_log_set_stdout(SysLogStdout out) {
    assert_param(out != NULL);
    sys_log.out = out;
}

void sys_log_set_uart() {
    sys_log_set_stdout(stdout_putf_uart);
    sys_log.custom_out = NULL;
}

void sys_log_set_swo() {
    HAL_UART_DeInit(bsp_uart_handler);
    sys_log_set_stdout(stdout_putf_swo);
    sys_log.custom_out = NULL;

}

void sys_log_set_custom(SysLogCustomOut stdout_proto) {
    sys_log.custom_out = stdout_proto;
}


void sys_log_print_str(const char* str) {
    printf_("%s", str);
}

void sys_log_print(SysLogLevel level, const char* tag, const char* format, ...) {
    if (sys_log.log_level < level) {
        return;
    }


    // Check if Protobuf logging is enabled
    if (sys_log.custom_out) {
        int formatted_length = 0;

        // Print the log header (timestamp, log level, tag)
        formatted_length = snprintf(log_message_buffer, sizeof(log_message_buffer),
                                    "%lu [%s] ", sys_log.timetamp(), tag);

        // Append the formatted message to the buffer
        va_list args;
        va_start(args, format);
        formatted_length += vsnprintf(log_message_buffer + formatted_length,
                                    sizeof(log_message_buffer) - formatted_length, format, args);
        va_end(args);

        // Ensure the buffer is null-terminated
        log_message_buffer[formatted_length + 1] = '\0';
        sys_log.custom_out(log_message_buffer);
    } else {
        // Output the log message normally if Protobuf is not enabled
        printf_("\r%lu %s[%s] " LOG_CLR_RESET, sys_log.timetamp(), log_colors[level], tag);
        va_list args;
        va_start(args, format);
        fctvsprintf(_stdout_putf, NULL, format, args);
        va_end(args);

    }
}



void sys_log_set_level(SysLogLevel level) {
    sys_log.log_level = level;
}

int16_t stdout_getc() {
    return sys_log.in();
}

void sys_log_init() {

    sys_log.log_level = LOG_LEVEL;
    sys_log.timetamp = HAL_GetTick;

    bsp_vcp_uart_init();
    sys_log_set_uart();

}