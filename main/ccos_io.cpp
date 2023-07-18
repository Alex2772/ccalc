#include <cstring>
#include <cstdarg>
#include "ccos_io.h"
#include "CCOSCore.h"


char _ccos_io_buf[64];
uint8_t _ccos_io_inf = 0;

void ccos_putchar(char c) {
    if (_ccos_io_inf <  64) {
        _ccos_io_buf[_ccos_io_inf++] = c;
    } else {
        for (size_t i = 0; i < 63; i++) {
            _ccos_io_buf[i] = _ccos_io_buf[i + 1];
        }
        _ccos_io_buf[63] = c;
    }
}

int ccos_readbuf(char *buf) {
    if (_ccos_io_inf) {
        memcpy(buf, ::_ccos_io_buf, _ccos_io_inf);
        int r = _ccos_io_inf;
        _ccos_io_inf = 0;
        return r;
    }
    return 0;
}

extern "C" {

int ccos_printf(const char *format, ...) {
    char buf[128] = {'\0'};

    va_list args;
    va_start(args, format);
    snprintf(buf, sizeof(buf), format, args);
    va_end(args);

    int l = strlen(buf);
    for (int i = 0; i < l; i++) {
        ccos_putchar(buf[i]);
    }
    return l;
}

}