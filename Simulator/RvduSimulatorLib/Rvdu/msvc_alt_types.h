#ifndef MSVC_ALT_TYPES
#define MSVC_ALT_TYPES

#include <stdio.h>
#include <crtdbg.h>
#include <stdarg.h>

typedef unsigned long long alt_u64;
typedef long long alt_64;

typedef unsigned int alt_u32;
typedef int alt_32;

typedef unsigned short alt_u16;
typedef short alt_16;

typedef unsigned char alt_u8;
typedef char alt_8;

inline void alt_printf(const char* format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    _RPT1(_CRT_WARN, "%s", buffer);
    printf("%s", buffer);
}

#define fl_fopen fopen
#define fl_fgetc fgetc
#define fl_fread fread
#define fl_fclose fclose
#define fl_fseek fseek
#define fl_ftell ftell

extern alt_u16* TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ;
extern alt_u16* TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE;

#endif