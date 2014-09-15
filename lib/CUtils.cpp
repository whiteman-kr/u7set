#include "Stable.h"
#include <assert.h>
#include "../include/CUtils.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)
#endif
#include <QtWidgets/QDesktopWidget>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

CUtils::CUtils(void)
{
}


CUtils::~CUtils(void)
{
}

int CUtils::DpiX()
{
	static int dpiX = -1;
	if (dpiX == -1)
	{
		dpiX = QApplication::desktop()->screen()->logicalDpiX();
	}
	return dpiX;
}

int CUtils::DpiY()
{
	static int dpiY = -1;
	if (dpiY == -1)
	{
		dpiY = QApplication::desktop()->screen()->logicalDpiY();
	}
	return dpiY;
}


// Округление, числа до указанного количсетва знаков дробной части.
// mode указывает, округлять отбрасыыанием дробной части, или AwayFromZero, т.е. 1.5 -> 2.0
//
double CUtils::Round(double value, int digits, VFrame30::MidpointRounding mode)
{
	if (mode == VFrame30::MidpointRounding::ToFloor)
	{
		double intpart = 0.0;
		double fractpart = modf(value, &intpart);

		double temp = pow(10.0, digits);
		long long tempfract = (long long)(fractpart * temp);

		fractpart = tempfract / temp;
		return (intpart + fractpart);
	}
	else
	{
		return Round(value, digits);
	}
}

// Округление, числа до указанного количсетва знаков дробной части.
// 2.224 (2 зн.) -> 2.22, 2.5352 (2 зн) -> 2.54, 3.8 (0 зн.) -> 4
//
double CUtils::Round(double value, int digits)
{
	// value - значение для округления
	// digits - количество чисел дробной части в возвращаемом значении
	//
	double intpart = 0.0;
	double fractpart = modf(value, &intpart);

	if(digits != 0)
	{
		int bal = 0;		// Остаток от деления
		double temp = pow(10.0, digits + 1);
		long long tempfract = (long long)(fractpart * temp);

		if (std::abs(tempfract) < 10)
		{
			bal = (int)tempfract;
			tempfract /= 10;
		}

		if(std::abs(tempfract) >= 10)
		{
			bal = (int)(tempfract % 10);
			tempfract /= 10;
		}

		if(tempfract <= 0)
		{
			if (bal <= -5)
			{
				tempfract--;
				fractpart = tempfract / (temp / 10);
				return (intpart + fractpart);
			}
			else
			{
				fractpart = tempfract / (temp / 10);
				return (intpart + fractpart);
			}

		}
		else
		{
			if (tempfract >= 0)
			{
				if(bal >= 5)
				{
					tempfract++;
					fractpart = tempfract / (temp / 10);
					return (intpart + fractpart);
				}
				else
				{
					fractpart = tempfract / (temp / 10);
					return (intpart + fractpart);
				}
			}
		}
	}
	else
	{
		return Round(value);
	}

	return 0;
}

// Округление, до целого.
// 2.2 -> 2, 2.5 -> 3, 3.8 -> 4
//
double CUtils::Round(double value)
{
	// value - значение для округления
	//
	double intval = 0.0;
	double fract = modf(value, &intval);

	if(fract < 0)
	{
		if ((fract * 10) <= -5)
		{
			intval--;
			return intval;
		}
		else
		{
			return intval;
		}
	}

	if((fract * 10) >= 5)
	{
		intval++;
		return intval;
	}
	else
	{
		return intval;
	}
}

double CUtils::RoundDisplayPoint(double value)	// По сути floor, используется для приведения координат при SchemeUnit::Display
{
	return floor(value);
}

double CUtils::RoundPoint(double p, VFrame30::SchemeUnit unit)				// Округлить значение в зависимости от типа региональной метрики
{
	switch (unit)
	{
	case VFrame30::SchemeUnit::Display:
		assert(false);
		return 0;

	case VFrame30::SchemeUnit::Inch:
		return Round(p, InchesRoundDigits, VFrame30::MidpointRounding::AwayFromZero);

	case VFrame30::SchemeUnit::Millimeter:
		return Round(p, MillimetresRoundDigits, VFrame30::MidpointRounding::AwayFromZero);

	default:
		assert(false);
		return 0;
	}
}

double CUtils::ConvertPoint(double point, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo, VFrame30::ConvertDirection convertDirection)
{
	if (convertFrom == convertTo)
	{
		return point;
	}

	int dpi = convertDirection == VFrame30::ConvertDirection::Horz ? DpiX() : DpiY();
	if (dpi == 0)
	{
		assert(dpi != 0);
		return 0.0;
	}

	if (convertFrom == VFrame30::SchemeUnit::Display)
	{
		if (convertTo == VFrame30::SchemeUnit::Inch)
		{
			point = point / dpi;
			return point;
		}

		if (convertTo == VFrame30::SchemeUnit::Millimeter)
		{
			point = point * 25.4 / dpi;
			return point;
		}

		assert(false);
		return 0.0;
	}

	if (convertFrom == VFrame30::SchemeUnit::Inch)
	{
		if (convertTo == VFrame30::SchemeUnit::Display)
		{
			point = point * dpi;
			return point;
		}

		if (convertTo == VFrame30::SchemeUnit::Millimeter)
		{
			point = point * 25.4;
			return point;
		}

		assert(false);
		return 0.0;
	}

	if (convertFrom == VFrame30::SchemeUnit::Millimeter)
	{
		if (convertTo == VFrame30::SchemeUnit::Display)
		{
			point = point * dpi / 25.4;
			return point;
		}

		if (convertTo == VFrame30::SchemeUnit::Inch)
		{
			point = point / 25.4;
			return point;
		}

		assert(false);
		return 0.0;
	}

	assert(false);
	return 0.0;
}

void CUtils::ConvertPoint(double& x, double& y, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo)
{
	if (convertFrom == convertTo)
	{
		return;
	}

	int dpiX = DpiX();
	int dpiY = DpiY();

	if (dpiX == 0 || dpiY == 0)
	{
		assert(dpiX != 0);
		assert(dpiY != 0);
		return;
	}

	if (convertFrom == VFrame30::SchemeUnit::Display)
	{
		if (convertTo == VFrame30::SchemeUnit::Inch)
		{
			x = x / dpiX;
			y = y / dpiY;
			return;
		}

		if (convertTo == VFrame30::SchemeUnit::Millimeter)
		{
			x = x * 25.4 / dpiX;
			y = y * 25.4 / dpiY;
			return;
		}

		assert(false);
		return;
	}

	if (convertFrom == VFrame30::SchemeUnit::Inch)
	{
		if (convertTo == VFrame30::SchemeUnit::Display)
		{
			x = x * dpiX;
			y = y * dpiY;
			return;
		}

		if (convertTo == VFrame30::SchemeUnit::Millimeter)
		{
			x = x * 25.4;
			y = y * 25.4;
			return;
		}

		assert(false);
		return;
	}

	if (convertFrom == VFrame30::SchemeUnit::Millimeter)
	{
		if (convertTo == VFrame30::SchemeUnit::Display)
		{
			x = x * dpiX / 25.4;
			y = y * dpiY / 25.4;
			return;
		}

		if (convertTo == VFrame30::SchemeUnit::Inch)
		{
			x = x / 25.4;
			y = y / 25.4;
			return;
		}

		assert(false);
		return;
	}

	assert(false);
	return;
}

/// <summary>
/// Опредение пересечения двух отрезков
/// </summary>
/// <returns>true - отрезки пересекаются</returns>
bool CUtils::IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2)
{
	double v1 = (bx2 - bx1) * (ay1 - by1) - (by2 - by1) * (ax1 - bx1);
	double v2 = (bx2 - bx1) * (ay2 - by1) - (by2 - by1) * (ax2 - bx1);
	double v3 = (ax2 - ax1) * (by1 - ay1) - (ay2 - ay1) * (bx1 - ax1);
	double v4 = (ax2 - ax1) * (by2 - ay1) - (ay2 - ay1) * (bx2 - ax1);
	return (v1 * v2 < 0) && (v3 * v4 < 0);
}

/// <summary>
/// Определение, пересекает ли линия указанный прямоугольник
/// </summary>
bool CUtils::IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle)
{
	if (intersectRectangle.contains(ax1, ay1) == true ||
		intersectRectangle.contains(ax2, ay2) == true)
	{
		return true;
	}

	if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
										intersectRectangle.x(), intersectRectangle.y(),
										intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y()) == true)
	{
		return true;
	}

	if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
										intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y(),
										intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y() + intersectRectangle.height()) == true)
	{
		return true;
	}

	if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
										intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y() + intersectRectangle.height(),
										intersectRectangle.x(), intersectRectangle.y() + intersectRectangle.height()) == true)
	{
		return true;
	}

	if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
										intersectRectangle.x(), intersectRectangle.y() + intersectRectangle.height(),
										intersectRectangle.x(), intersectRectangle.y()) == true)
	{
		return true;
	}

	return false;
}

// Вычисление хэш кода имени класса, сохраняется в сериализуемом файле, не изменять способ вычисления,
// данные уже сохранены в файл.
// Имя класса может содеражать namespace, но он будет отрезан
// т.е. VFrame30::CVideoFrame будет оьбрезано до CVideoFrame
//
quint32 CUtils::GetClassHashCode(const std::string& className)
{
	assert(className.empty() == false);

	std::string clearClassName = className;

	auto findResult = className.rfind("::");
	if (findResult != className.npos)
	{
		assert(findResult + 2 < className.size());
		assert((findResult + 2) + (className.size() - findResult - 2) == className.size());

		clearClassName = className.substr(findResult + 2, className.size() - findResult - 2);
	}

	quint32 nHash = 0;
	const char* ptr = clearClassName.c_str();

	while (*ptr)
		nHash += (nHash<<5) + *ptr++;

	return nHash;
}

