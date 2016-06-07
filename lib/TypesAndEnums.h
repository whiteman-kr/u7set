#ifndef TYPESANDENUMS_H
#define TYPESANDENUMS_H

namespace VFrame30
{
	// SchemaUnit
	//
	enum class SchemaUnit
	{
		Display,			// display pixels
		Millimeter,			// mm
		Inch				// inches
	};

	// Conversion directiun, by this DPI can be detected (dpi X/ dpi Y)
	//
	enum ConvertDirection
	{
		Horz,
		Vert
	};

	// Тип округления
	//
	enum MidpointRounding
	{
		ToFloor,			// Floor rounding
		AwayFromZero		// When a number is halfway between two others, it is rounded toward the nearest number that is away from zero.
	};
}

#endif
