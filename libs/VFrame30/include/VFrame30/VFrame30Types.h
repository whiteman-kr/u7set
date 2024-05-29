#pragma once

namespace VFrame30
{
	enum class ZoomMode
	{
		Manual,           // Zoom is fully manual.
		Always100Percent, // View is always zoomed to 100% (like Monitor 2.0).
		FitToScreen       // View is always zoomed to fit the Monitor window.
	};

	enum class DrawMode
	{
		Monitor,
		Simulator,
		Editor
	};
} // namespace VFrame