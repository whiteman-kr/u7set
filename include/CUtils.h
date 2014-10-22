#pragma once

#include "TypesAndEnums.h"

// Перевести миллиметры в дюймы
//
#define mm2in(mmVal) (mmVal / 25.4)

// Перевести дюймы в миллиметры
//
#define in2mm(inVal) (inVal * 25.4)

// Ассерт при наступлении условия
//
#define ASSERT_IF(expr) std::assert(!(expr))

class CUtils
{
public:
	CUtils(void);
	~CUtils(void);

	// --
	//
	static const int InchesRoundDigits = 4;			// Количество знаков после запятой для округления для дюймов
	static const int MillimetresRoundDigits = 2;	// Количество знаков после запятой для округления для миллиметров

	static int DpiX();								// Получить DpiX для дисплея, инициализируется на первом вызове
	static int DpiY();								// Получить DpiY для дисплея

	// Math functions
	//

	static double Round(double value, int digits, VFrame30::MidpointRounding mode);	// Округление, числа до указанного количсетва знаков дробной части.
	static double Round(double value, int digits);	// Округление, числа до указанного количсетва знаков дробной части.
	static double Round(double value);				// Округление, до целого.

	static double RoundDisplayPoint(double value);			// По сути floor, используется для приведения координат при SchemeUnit::Display
	static double RoundPoint(double p, VFrame30::SchemeUnit unit);	// Округлить значение в зависимости от типа региональной метрики

	static double ConvertPoint(double point, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo, VFrame30::ConvertDirection convertDirection);
	static void ConvertPoint(double& x, double& y, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo);

	static QPointF snapToGrid(const QPointF& pt, double gridSize);
	static QPointF snapToGrid(double x, double y, double gridSize);
	static double snapToGrid(double value, double gridSize);

	/// <summary>
	/// Опредение пересечения двух отрезков
	/// </summary>
	/// <returns>true - отрезки пересекаются</returns>
	static bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2);

	/// <summary>
	/// Определение, пересекает ли линия указанный прямоугольник
	/// </summary>
	static bool IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle);

	/// <summary>
	/// Вычисление хэш кода имени класса
	/// </summary>
	/// <remarks>
	/// Сохраняется в сериализуемом файле, не изменять способ вычисления
	/// </remarks>
	static quint32 GetClassHashCode(const std::string& className);

};



