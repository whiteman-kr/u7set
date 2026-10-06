#ifndef RN_FAT_FILELIB_H
#define RN_FAT_FILELIB_H

#ifndef RVDU_SIM        

#error This file is for simulator!

#else

// Simulator-specific

#include <stdio.h>

#define fl_fopen fopen
#define fl_fgetc fgetc
#define fl_fread fread
#define fl_fclose fclose
#define fl_fseek fseek
#define fl_ftell ftell

// Simulator-specific

#endif

#endif
