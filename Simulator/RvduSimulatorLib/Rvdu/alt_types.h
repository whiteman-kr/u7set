#ifndef ALT_TYPES_H
#define ALT_TYPES_H

#ifndef RVDU_SIM        

#error This file is for simulator!

#else

typedef unsigned long long alt_u64;
typedef long long alt_64;

typedef unsigned int alt_u32;
typedef int alt_32;

typedef unsigned short alt_u16;
typedef short alt_16;

typedef unsigned char alt_u8;
typedef char alt_8;

#endif

#endif