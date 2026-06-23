#pragma once

#include <cstdint>

// Saving TrendItem Signals data to file File::VDU_TREND_SIGNALS
// File has the following structure:
//
//		TrendItemSignalsHeader
//		TrendItemSignal[TrendItemSignalsHeader.count]
//		CRC64 of the file excluding CRC64 itself
//

// Pack structs to 1 byte alignment
//
#pragma pack(push, 1)

struct TrendItemSignalsHeader
{
	uint32_t version; // 1
	uint32_t recordSize;
	uint32_t count;
	uint32_t reserve;
};

struct TrendItemSignal
{
	uint32_t appSignalIndex;
	uint32_t validityAppSignalIndex; // 0xFFFFFFFF if no validity signal is used
	uint32_t durationSecs;
	uint32_t reserve;

	auto operator<=>(const TrendItemSignal&) const = default;
};

#pragma pack(pop)