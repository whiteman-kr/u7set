// GwCrc32.hpp
//
#pragma once

#include <span>
#include <cstdint>

/*
    Name              : CRC-32
    Poly              : 0x04C11DB7 (reflected: 0xEDB88320)
    Input reflection  : yes
    Output reflection : yes
    Init              : 0xFFFFFFFF
    XorOut            : 0xFFFFFFFF
    Check             : 0xCBF43926 ("123456789")
*/

namespace Radiy 
{
    constexpr uint32_t Crc32Residue = 0x2144DF1C;   // Expected residue when appending CRC to data
    constexpr uint32_t Crc32Init = 0xFFFFFFFF;      // Initial CRC value
    constexpr uint32_t Crc32FinalXor = 0xFFFFFFFF;  // Final XOR value

    /**
     * Calculates the CRC-32 checksum for the given data.
     *
     * @param data Input data as a span of bytes.
     * @param finalize Whether to finalize the CRC calculation (default: true).
     * @param initialCrc Initial CRC value (default: Crc32Init).
     * @return The computed CRC-32 checksum.
     */
    uint32_t CRC32(std::span<const std::byte> data, bool finalize = true, uint32_t initialCrc = Crc32Init);

    /**
     * Convenience overload for char data (text/strings).
     *
     * @param data Input data as a span of char.
     * @param finalize Whether to finalize the CRC calculation (default: true).
     * @param initialCrc Initial CRC value (default: Crc32Init).
     * @return The computed CRC-32 checksum.
     */
    uint32_t CRC32(std::span<const char> data, bool finalize = true, uint32_t initialCrc = Crc32Init);

    /**
     * C-style interface with pointer and size.
     *
     * @param data Pointer to the input data buffer.
     * @param length Length of the input data buffer in bytes.
     * @param finalize Whether to finalize the CRC calculation (default: true).
     * @param initialCrc Initial CRC value (default: Crc32Init).
     * @return The computed CRC-32 checksum.
     */
    uint32_t CRC32(const char* data, size_t length, bool finalize = true, uint32_t initialCrc = Crc32Init);
} // namespace Radiy