#include "sys_console.h"
#include "sys_log.h"
#include "stdbool.h"
#include "sys_compiler.h"
#include "sys_descriptor.h"

#include <string.h>
#include <ctype.h>


#define SHELL_RX_BUFFER_SIZE (128)
#define SHELL_MAX_ARGS (4)

#define SHELL_PROMPT "\r" LOG_CLR_I "$ " LOG_CLR_D PROJECT_NAME " > " LOG_CLR_RESET

#define SHELL_FOR_EACH_COMMAND(command)                        \
    for(const sShellCommand* command = g_console_commands;     \
        command < &g_console_commands[g_num_console_commands]; \
        ++command)


static struct ShellContext {
    int (*send_char)(char c);
    size_t rx_size;
    char rx_buffer[SHELL_RX_BUFFER_SIZE];
    char history[SHELL_RX_BUFFER_SIZE];
} s_shell;

int sys_console_putc(char c) {
    stdout_putf(c);
    return 1;
}

int16_t sys_console_getc(void) {
    return stdout_getc();
}

static bool prv_booted(void) {
    return s_shell.send_char != NULL;
}

static void prv_send_char(char c) {
    if(!prv_booted()) {
        return;
    }
    s_shell.send_char(c);



}

static void prv_echo(char c) {
    if('\n' == c) {
        prv_send_char('\r');
        prv_send_char('\n');
    } else if('\b' == c) {
        prv_send_char('\b');
        prv_send_char(' ');
        prv_send_char('\b');
    } else {
        prv_send_char(c);
    }

}

static char prv_last_char(void) {
    return s_shell.rx_buffer[s_shell.rx_size - 1];
}

static bool prv_is_rx_buffer_full(void) {
    return s_shell.rx_size >= SHELL_RX_BUFFER_SIZE;
}

static void prv_reset_rx_buffer(void) {
    memset(s_shell.rx_buffer, 0, sizeof(s_shell.rx_buffer));
    s_shell.rx_size = 0;
}

static void prv_echo_str(const char* str) {
    for(const char* c = str; *c != '\0'; ++c) {
        prv_echo(*c);
    }
}

static void prv_send_prompt(void) {
    prv_echo_str(SHELL_PROMPT);
}

static const sShellCommand* prv_find_command(const char* name) {
    SHELL_FOR_EACH_COMMAND(command) {
        const size_t str_len = strlen(command->command);
        if(memcmp(command->command, name, str_len) == 0 &&
           strlen(command->command) == strlen(name)) {
            return command;
        }
    }
    return NULL;
}

static void prv_process(void) {

    if(s_shell.rx_buffer[s_shell.rx_size - 3] == '\x1b') {

        memcpy(s_shell.rx_buffer, s_shell.history, SHELL_RX_BUFFER_SIZE);
        s_shell.rx_size = strlen(s_shell.rx_buffer) + 1;

        prv_echo('\r');
        prv_send_prompt();
        prv_echo_str(s_shell.history);

#if 0
        if(s_shell.rx_buffer[s_shell.rx_size - 2] == 171){ // up
            return;
        } else if(s_shell.rx_buffer[s_shell.rx_size - 2] == 43){ // down
            return;
        }
#endif 
        return;
    }

    if(prv_last_char() != KEY_ENTER && !prv_is_rx_buffer_full()) {
        return;
    }

    char* argv[SHELL_MAX_ARGS] = {0};
    int argc = 0;

    char* next_arg = NULL;
    for(size_t i = 0; i < s_shell.rx_size && argc < SHELL_MAX_ARGS; ++i) {
        char* const c = &s_shell.rx_buffer[i];
        if(*c == ' ' || *c == '\n' || i == s_shell.rx_size - 1) {
            *c = '\0';
            if(next_arg) {
                argv[argc++] = next_arg;
                next_arg = NULL;
            }
        } else if(!next_arg) {
            next_arg = c;
        }
    }

    if(s_shell.rx_size == SHELL_RX_BUFFER_SIZE) {
        prv_echo('\n');
    }

    if(argc >= 1) {
        const sShellCommand* command = prv_find_command(argv[0]);

        if(!command) {
            log_e("Error", "Unknown command: %s", argv[0]);
            log_e("Error", "Type 'help' to list all commands", argv[0]);
        } else {
            command->handler(command->command, argc, argv);
        }

        memcpy(s_shell.history, s_shell.rx_buffer, SHELL_RX_BUFFER_SIZE);
        
    }
    


    prv_reset_rx_buffer();
    prv_send_prompt();
}

void sys_console_prompt() {
    prv_echo_str("\r\n" SHELL_PROMPT);
}

void sys_console_init(const char* splash) {
    s_shell.send_char = sys_console_putc;
    prv_reset_rx_buffer();

    prv_echo_str("\033[2J");
    prv_echo_str("\r\n");
    
    if(splash != NULL){
        prv_echo_str(splash);
    }

    sys_console_prompt();

}

void sys_console_rx(char c) {

    if(prv_is_rx_buffer_full() || !prv_booted()) {
        return;
    }

    if(c == KEY_BACKSPACE && s_shell.rx_size != 0) { // handle backspace
        memset(&s_shell.rx_buffer[s_shell.rx_size], 0, SHELL_RX_BUFFER_SIZE - s_shell.rx_size);
        s_shell.rx_buffer[s_shell.rx_size - 1] = '\0';
        s_shell.rx_size = strlen(s_shell.rx_buffer);

        prv_echo(c);

        if(s_shell.rx_size == 0){
            prv_echo_str("\r" SHELL_PROMPT);
            prv_echo('\0');
        }

        return;

    } else {
        s_shell.rx_buffer[s_shell.rx_size++] = c;
    }
        
    if(isprint(c)) {
        prv_echo(c);
    }

    prv_process();
    
}

void sys_console_put_line(const char* str) {
    prv_echo_str(str);
    prv_echo('\n');
}

int sys_console_help_handler(const char* name, int argc, char* argv[]) {
    prv_echo('\n');
    SHELL_FOR_EACH_COMMAND(command) {
        prv_echo_str(command->command);
        prv_echo_str(":\t\t");
        prv_echo_str(command->help);
        prv_echo('\n');
    }
    return 0;
}