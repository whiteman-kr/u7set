#ifndef TYPESANDENUMS_H
#define TYPESANDENUMS_H

namespace VFrame30
{
	// SchemeUnit
	//
	enum SchemeUnit
	{
		Display,			// измерение в точках устройства (пиксели для дисплея)
		Millimeter,			// измерение в мм
		Inch				// измерение в дюймах
	};

	// Направление конвертирования координат, по этому типу определяется какой Dpi (DpiX/DpiY) использовать
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
