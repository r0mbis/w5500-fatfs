#pragma once

#include <stddef.h>
#include <stdint.h>

#define KEY_UP "\x1b\x5b\x41" /* [up] key: 0x1b 0x5b 0x41 */
#define KEY_DOWN "\x1b\x5b\x42" /* [down] key: 0x1b 0x5b 0x42 */
#define KEY_RIGHT "\x1b\x5b\x43" /* [right] key: 0x1b 0x5b 0x43 */
#define KEY_LEFT "\x1b\x5b\x44" /* [left] key: 0x1b 0x5b 0x44 */
#define KEY_ENTER '\r' /* [enter] key */
#define KEY_BACKSPACE '\b' /* [backspace] key */
#define KEY_DEL '\x7f' /* [DEL] key */
#define KEY_DELETE "\x1b\x5b\x33\x7e" /*[Delete] key */

enum {
    E_FONT_BLACK,
    E_FONT_L_RED,
    E_FONT_RED,
    E_FONT_GREEN,
    E_FONT_YELLOW,
    E_FONT_BLUE,
    E_FONT_PURPLE,
    E_FONT_CYAN,
    E_FONT_WHITE,
};

#define PRINTF_COLOR(c, ...)                                           \
    _Pragma("GCC warning \"'PRINTF_COLOR' macro is deprecated\"") do { \
        switch(c) {                                                    \
        case E_FONT_BLACK:                                             \
            TERMINAL_FONT_BLACK();                                     \
            break;                                                     \
        case E_FONT_L_RED:                                             \
            TERMINAL_FONT_L_RED();                                     \
            break;                                                     \
        case E_FONT_RED:                                               \
            TERMINAL_FONT_RED();                                       \
            break;                                                     \
        case E_FONT_GREEN:                                             \
            TERMINAL_FONT_GREEN();                                     \
            break;                                                     \
        case E_FONT_YELLOW:                                            \
            TERMINAL_FONT_YELLOW();                                    \
            break;                                                     \
        case E_FONT_BLUE:                                              \
            TERMINAL_FONT_BLUE();                                      \
            break;                                                     \
        case E_FONT_PURPLE:                                            \
            TERMINAL_FONT_PURPLE();                                    \
            break;                                                     \
        case E_FONT_CYAN:                                              \
            TERMINAL_FONT_CYAN();                                      \
            break;                                                     \
        case E_FONT_WHITE:                                             \
            TERMINAL_FONT_WHITE();                                     \
            break;                                                     \
        }                                                              \
        printf_(__VA_ARGS__);                                          \
        TERMINAL_FONT_DEFAULT();                                       \
    }                                                                  \
    while(0)

/* terminal display-----------------------------------------------------BEGIN */

/*
    @links: http://blog.csdn.net/yangguihao/article/details/47734349
            http://blog.csdn.net/kevinshq/article/details/8179252


    @terminal setting commands:
        \033[0m     reset all
        \033[1m     set high brightness
        \03[4m      underline
        \033[5m     flash
        \033[7m     reverse display
        \033[8m     blanking
        \033[30m    --  \033[37m  set font color
        \033[40m    --  \033[47m  set background color
        \033[nA     cursor up up n lines
        \033[nB     cursor move up n lines
        \033[nC     cursor move right n lines
        \033[nD     cursor left up n lines
        \033[y;xH   set cursor position
        \033[2J     clear all display
        \033[K      clear line
        \033[s      save cursor position
        \033[u      restore cursor position
        \033[?25l   cursor invisible
        \33[?25h    cursor visible


    @background color: 40--49           @font color: 30--39
        40: BLACK                           30: black
        41: RED                             31: red
        42: GREEN                           32: green
        43: YELLOW                          33: yellow
        44: BLUE                            34: blue
        45: PURPLE                          35: purple
        46: CYAN                            36: deep green
        47: WHITE                           37: white
*/

/* font color */

#define CLI_FONT_BLACK "\033[1;30m"
#define CLI_FONT_L_RED "\033[0;31m" /* light red */
#define CLI_FONT_RED "\033[1;31m" /* red */
#define CLI_FONT_GREEN "\033[1;32m"
#define CLI_FONT_YELLOW "\033[1;33m"
#define CLI_FONT_BLUE "\033[1;34m"
#define CLI_FONT_PURPLE "\033[1;35m"
#define CLI_FONT_CYAN "\033[1;36m"
#define CLI_FONT_WHITE "\033[1;37m"
#define CLI_FONT_GREY "\033[1;90m"
#define CLI_FONT_DEFAULT CLI_FONT_WHITE

#define TERMINAL_FONT_BLACK "\033[1;30m"
#define TERMINAL_FONT_L_RED "\033[0;31m" /* light red */
#define TERMINAL_FONT_RED "\033[1;31m" /* red */
#define TERMINAL_FONT_GREEN "\033[1;32m"
#define TERMINAL_FONT_YELLOW "\033[1;33m"
#define TERMINAL_FONT_BLUE "\033[1;34m"
#define TERMINAL_FONT_PURPLE "\033[1;35m"
#define TERMINAL_FONT_CYAN "\033[1;36m"
#define TERMINAL_FONT_WHITE "\033[1;37m"
#define TERMINAL_FONT_DEFAULT TERMINAL_FONT_WHITE()

/* background color */
#define CLI_BACK_BLACK "\033[1;40m"
#define CLI_BACK_L_RED "\033[0;41m" /* light red */
#define CLI_BACK_RED "\033[1;41m" /* red */
#define CLI_BACK_GREEN "\033[1;42m"
#define CLI_BACK_YELLOW "\033[1;43m"
#define CLI_BACK_BLUE "\033[1;44m"
#define CLI_BACK_PURPLE "\033[1;45m"
#define CLI_BACK_CYAN "\033[1;46m"
#define CLI_BACK_WHITE "\033[1;47m"
#define CLI_BACK_DEFAULT CLI_BACK_BLACK

#define TERMINAL_BACK_BLACK() printf_("\033[1;40m")
#define TERMINAL_BACK_L_RED() printf_("\033[0;41m") /* light red */
#define TERMINAL_BACK_RED() printf_("\033[1;41m") /* red */
#define TERMINAL_BACK_GREEN() printf_("\033[1;42m")
#define TERMINAL_BACK_YELLOW() printf_("\033[1;43m")
#define TERMINAL_BACK_BLUE() printf_("\033[1;44m")
#define TERMINAL_BACK_PURPLE() printf_("\033[1;45m")
#define TERMINAL_BACK_CYAN() printf_("\033[1;46m")
#define TERMINAL_BACK_WHITE() printf_("\033[1;47m")
#define TERMINAL_BACK_DEFAULT() TERMINAL_BACK_BLACK()

/* terminal clear end */
#define TERMINAL_CLEAR_END "\033[K"

/* terminal clear all */
#define TERMINAL_DISPLAY_CLEAR "\033[2J"

/* cursor move up */
#define TERMINAL_MOVE_UP(x)                 \
    do {                                    \
        if(x > 0) printf_("\033[%dA", (x)); \
    } while(0)

/* cursor move down */
#define TERMINAL_MOVE_DOWN(x)               \
    do {                                    \
        if(x > 0) printf_("\033[%dB", (x)); \
    } while(0)

/* cursor move left */
#define TERMINAL_MOVE_LEFT(y)               \
    do {                                    \
        if(y > 0) printf_("\033[%dD", (y)); \
    } while(0)

/* cursor move right */
#define TERMINAL_MOVE_RIGHT(y)              \
    do {                                    \
        if(y > 0) printf_("\033[%dC", (y)); \
    } while(0)

/* cursor move to */
#define TERMINAL_MOVE_TO(x, y) "\033[%d;%dH", (x), (y)

/* cursor reset */
#define TERMINAL_RESET_CURSOR "\033[H"

/* cursor invisible */
#define TERMINAL_HIDE_CURSOR "\033[?25l"

/* cursor visible */
#define TERMINAL_SHOW_CURSOR "\033[?25h"

/* reverse display */
#define TERMINAL_HIGHLIGHT "\033[7m"
#define TERMINAL_UN_HIGHLIGHT "\033[27m"

typedef struct ShellCommand {
    const char* command;
    int (*handler)(const char* name, int argc, char* argv[]);
    const char* help;
} sShellCommand;

extern const sShellCommand* const g_console_commands;
extern const size_t g_num_console_commands;

//! Initializes the demo shell. To be called early at boot.
void sys_console_init(const char* splash);
void sys_console_prompt();
//! Call this when a character is received. The character is processed synchronously.
void sys_console_rx(char c);

//! Print help command
int sys_console_help_handler(const char* name, int argc, char* argv[]);

//! Prints a line then a newline
void sys_console_put_line(const char* str);

int sys_console_putc(char c);
int16_t sys_console_getc(void);