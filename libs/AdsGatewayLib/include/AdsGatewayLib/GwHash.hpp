#pragma once
#include <cstdint>
#include <string_view>

namespace Radiy
{
	using Hash = uint64_t;

	inline constexpr Hash UNDEFINED_HASH = 0x0000000000000000ULL;

	/**
	 * Calculate hash for AppSignalID
	 *
	 * @param str - String view of AppSignalID (must start with '#', ASCII characters only)
	 * @param init - Initial hash value (default: 0)
	 * @return Hash - 64-bit hash value
	 *
	 * Example:
	 *   std::string_view signalId = "#APPSIGNALID_1SF";
	 *   Hash hash = calcHash(signalId);
	 */
	[[nodiscard]] constexpr Hash calcHash(std::string_view str, Hash init = 0ULL)
	{
		Hash hash = init;

		for (char c : str)
		{
			uint16_t unicode = static_cast<unsigned char>(c);
			hash += (hash << 5) + unicode;
		}

		return hash;
	}
} // namespace Radiy