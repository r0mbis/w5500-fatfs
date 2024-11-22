
#include <stdint.h>
#include <stdio.h>

__attribute__((weak)) void* __wrap__malloc_r(struct _reent* r, size_t size) {
    return NULL;
}

__attribute__((weak)) void __wrap__free_r(struct _reent* r, void* ptr) {
}

__attribute__((weak)) void* __wrap__calloc_r(struct _reent* r, size_t count, size_t size) {
    return NULL;
}

__attribute__((weak)) void* __wrap__realloc_r(struct _reent* r, void* ptr, size_t size) {
    return NULL;
}

__attribute__((weak)) int _getpid(void* st) {
    return 0;
}
__attribute__((weak)) int _isatty(int fd) {
    return 0;
}
__attribute__((weak)) int _close(int fd) {
    return 0;
}
__attribute__((weak)) int _lseek(int fd, int ptr, int dir) {
    return 0;
}
__attribute__((weak)) int _read(int fd, char* ptr, int len) {
    return 0;
}
__attribute__((weak)) int _fstat(int fd, void* st) {
    return 0;
}
__attribute__((weak)) int _kill(void* st) {
    return 0;
}

__attribute__((weak))int _init(void* st) {
    return 0;
}

__attribute__((weak)) int _write(void* st) {
    return 0;
}

__attribute__((weak)) int __io_putchar(int ch) {
    return ch;
}
