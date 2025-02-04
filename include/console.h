#pragma once

#include <stddef.h>

static inline void
console_log(char buffer[])
{
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

char *tohex(unsigned long num, char buffer[]);
char *ulltoa(size_t num, char buffer[]);
