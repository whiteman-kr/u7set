#pragma once

#include <stdbool.h>
#include <stdint.h>

// vdu_string_ref is an offset in a file to UTF16 string (QChar).
// String starts from the string size (16-bit), then follows string data (16-bit QChars).
// The string is a null terminated.
// Padding and alignment is 4 bytes.
//
typedef uint32_t vdu_string_ref;
typedef uint32_t vdu_file_ref;

inline const vdu_string_ref StringRefStub = 0x52525453; // "STRR" - for debug, easy to find in hex editor.

// vdu_cstr is an offset in a file to UTF8 string
// String starts from the string size (16-bit), then follows string data.
// The string is a null terminated.
// Padding and alignment is 4 bytes.
//
typedef uint32_t vdu_cstr;

// vdu_scriptbc is a Lua script bytecode.
// Data starts from 32-bit size, then follows bytecode.
// Padding and alignment is 4 bytes.
//
typedef uint32_t vdu_scriptbc;

inline const vdu_scriptbc LuaBytecodeRefStub = 0x4342544C; // "LBTC" - for debug, easy to find in hex editor.

// --
//
inline const int VDU_OPTO_PORTS_COUNT = 8;