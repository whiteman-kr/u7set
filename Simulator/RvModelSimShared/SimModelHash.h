#pragma once

#include <stdint.h>
#include <stddef.h>

extern const int16_t crc16tab[];

// Обчислення хешу
//
typedef uint64_t Hash;
typedef uint32_t Hash32;
#define UNDEFINED_HASH 0x0000000000000000ULL

// Функція розрахунку CRC16 (x^16 + x^12 + x^2 + 1)
//
uint16_t calcCrc16(const void* buf, int len);

// Хеш-функція для латинських ASCII/UTF-8 символів
//
Hash calcHash(const char* str);

