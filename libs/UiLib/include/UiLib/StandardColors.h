#pragma once
#include <QColor>

namespace StandardColors
{
	static constexpr QRgb VcsCheckedIn{qRgb(0xFF, 0xFF, 0xFF)};		// NOLINT
	static constexpr QRgb VcsAdded{qRgb(0xE0, 0xFF, 0xE0)};			// NOLINT
	static constexpr QRgb VcsModified{qRgb(0xE0, 0xE8, 0xFF)};		// NOLINT
	static constexpr QRgb VcsDeleted{qRgb(0xFF, 0xE0, 0xE0)};		// NOLINT

	static constexpr QRgb ExcludedFromBuildForeground{qRgb(0xA0, 0xA0, 0xA0)};

	static constexpr QRgb LogWarningForeground{qRgb(0xF8, 0x72, 0x17)};
	static constexpr QRgb LogErrorForeground{qRgb(0xFF, 0x00, 0x00)}; 
}
