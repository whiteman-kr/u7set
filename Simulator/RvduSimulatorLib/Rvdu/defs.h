#ifndef DEFS_H
#define DEFS_H

#include <alt_types.h>

#define INVALID_SIGNAL_INDEX (alt_u16)-1

// String lengths
//
#define STRING_MAX_LEN 256u
#define FILE_NAME_MAX_LEN 256u
#define BUTTON_TEXT_MAX_LEN 32u

#define MAX_SCREEN_WIDTH  4096u
#define MAX_SCREEN_HEIGHT  2048u

// TODO: later, move these constants to PROJECT Hardware configuration
//
#define RVDU_SCREEN_WIDTH 1280u
#define RVDU_SCREEN_HEIGHT 720u

// File versions
//
#define FILE_CFG_VERSION 1u
#define FILE_SCHEMA_VERSION 1u
#define FILE_SIGNALS_VERSION 1u

#endif