#include "SEGGER_RTT.h"
#include <stdio.h>

int _write(int file, char *ptr, int len) {
    (void)file;
    int written = SEGGER_RTT_Write(0, ptr, len);
    return written;
}

