#include <stddef.h>

#include <string.h>
#include <unistd.h>

#include "block.h"
#include "console.h"

char *
ulltoa(size_t num, char buffer[])
{
    int i = 0;
    do {
        buffer[i++] = num % 10 + '0';
        num /= 10;
    } while (num);

    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    buffer[i] = '\0';

    return buffer;
}

char *
tohex(unsigned long num, char buffer[])
{
    int i = 0;

    do {
        buffer[i++] = "0123456789ABCDEF"[num & 0xF];
        num >>= 4;
    } while(num);

    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    buffer[i] = '\0';

    return buffer;
}
