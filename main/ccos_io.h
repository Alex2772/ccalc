//
// Created by alex2772 on 16.12.17.
//

#ifndef CCOS_IO_H
#define CCOS_IO_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
void ccos_putchar(char c);
int ccos_printf(const char* fmt, ...);
int ccos_readbuf(char* buf);

#ifdef __cplusplus
};
#endif
#endif