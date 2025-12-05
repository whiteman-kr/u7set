#pragma once

#include <cstdint>
#include <string_view>

using Hash = uint64_t;

#define UNDEFINED_HASH 0x0000000000000000ULL // Do not change to other value.

// Constexpr version for std::string_view
// Note: Assumes ASCII/Latin-1 input where each byte maps directly to UTF-16 code unit
// For full UTF-8 support, conversion to UTF-16 would be needed (not constexpr-friendly)
//
inline Hash calcHash(std::string_view str, Hash init = 0)
{
	Hash hash = init;

	for (char c : str)
	{
		// Cast to uint16_t to match QChar::unicode() behavior
		// This works correctly for ASCII and Latin-1 (0-255)
		//
		uint16_t unicode = static_cast<unsigned char>(c);
		hash += (hash << 5) + unicode;
	}

	return hash;
}