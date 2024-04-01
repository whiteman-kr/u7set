#pragma once

#include <stdbool.h>
#include <stdint.h>

// string_ref is an offset in a file to a string.
// String consist of 16 bit size of string in symbols, followed with string data. Padding to 4 bytes.
// The string is a null terminated QChar string.
// (In Qt, Unicode characters are 16-bit entities without any markup or structure).
// Note: String in file must be aligned to 4 bytes.
//
typedef uint32_t vdu_string_ref;

inline const int VDU_OPTO_CONNECTIONS_COUNT = 8;
