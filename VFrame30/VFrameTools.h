#pragma once

// Even though it is included by public precompile header, cmake puts it after this file, so it is needed to include it here
//
#include "../CommonLib/Types.h"

namespace VFrame30
{
	[[maybe_unused]] static constexpr int InchesRoundDigits = 4;		// How many digits should be left for pretty look
	[[maybe_unused]] static constexpr int MillimetresRoundDigits = 2;	// How many digits should be left for pretty look

	// Round type
	//
	enum class MidpointRounding
	{
		ToFloor,			// Floor rounding
		AwayFromZero		// When a number is halfway between two others, it is rounded toward the nearest number that is away from zero.
	};


	// Rounding functions
	//

	// Round to even
	//
	double Round(double value);

	// Rounding to 'digits' after point
	//
	double Round(double value, int digits);

	// Rounding number to 'digits' decimal places after point
	// mode - fraction or AwayFromZero, e.g. 1.5 -> 2.0
	//
	double Round(double value, int digits, MidpointRounding mode);

	// This is floor for SchemaUnit::Display
	//
	double RoundDisplayPoint(double value);

	// Round value dpending on regional metrics
	//
	double RoundPoint(double p, SchemaUnit unit);

	//
	// Convert functions
	//
	double mm2in(double mmVal);
	double in2mm(double inVal);

	double ConvertPoint(double point, SchemaUnit convertFrom, SchemaUnit convertTo, double dpi);
	void ConvertPoint(double& x, double& y, const SchemaUnit convertFrom, const SchemaUnit convertTo, const double dpiX, const double dpiY);

	QPointF snapToGrid(QPointF pt, double gridSize);
	QPointF snapToGrid(double x, double y, double gridSize);
	double snapToGrid(const double value, const double gridSize);

	// Detect two lines intersection
	//
	bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2);

	// Detect line and rect intersection
	//
	bool IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle);
}
