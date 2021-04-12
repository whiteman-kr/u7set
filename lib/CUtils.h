#pragma once

#include "TypesAndEnums.h"
#include "../UtilsLib/Hash.h"

#include <QtWidgets/QApplication>
#include <assert.h>
#include <cmath>
#include <QDateTime>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)
#endif
#include <QtWidgets/QDesktopWidget>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define mm2in(mmVal) (mmVal / 25.4)
#define in2mm(inVal) (inVal * 25.4)

class CUtils
{
public:

	static const int InchesRoundDigits = 4;
	static const int MillimetresRoundDigits = 2;

	// Math functions
	//

	// Округление, числа до указанного количсетва знаков дробной части.
	// mode указывает, округлять отбрасыыанием дробной части, или AwayFromZero, т.е. 1.5 -> 2.0
	//
	static double Round(double value, int digits, VFrame30::MidpointRounding mode)	// Округление, числа до указанного количсетва знаков дробной части.
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
	static double Round(double value, int digits)
	{
		// Округление, числа до указанного количсетва знаков дробной части.
		// 2.224 (2 зн.) -> 2.22, 2.5352 (2 зн) -> 2.54, 3.8 (0 зн.) -> 4
		//
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
	static double Round(double value)
	{
		// Округление, до целого.
		// 2.2 -> 2, 2.5 -> 3, 3.8 -> 4
		//

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

	// По сути floor, используется для приведения координат при SchemaUnit::Display
	static double RoundDisplayPoint(double value)
	{
		return floor(value);
	}

	// Округлить значение в зависимости от типа региональной метрики
	static double RoundPoint(double p, VFrame30::SchemaUnit unit)
	{
		switch (unit)
		{
		case VFrame30::SchemaUnit::Display:
			assert(false);
			return 0;

		case VFrame30::SchemaUnit::Inch:
			return Round(p, InchesRoundDigits, VFrame30::MidpointRounding::AwayFromZero);

		case VFrame30::SchemaUnit::Millimeter:
			return Round(p, MillimetresRoundDigits, VFrame30::MidpointRounding::AwayFromZero);

		default:
			assert(false);
			return 0;
		}
	}

	static double ConvertPoint(double point, VFrame30::SchemaUnit convertFrom, VFrame30::SchemaUnit convertTo, int dpi)
	{
		if (convertFrom == convertTo)
		{
			return point;
		}

		if (convertFrom == VFrame30::SchemaUnit::Display)
		{
			if (dpi == 0)
			{
				assert(dpi != 0);
				return 0.0;
			}

			if (convertTo == VFrame30::SchemaUnit::Inch)
			{
				point = point / dpi;
				return point;
			}

			if (convertTo == VFrame30::SchemaUnit::Millimeter)
			{
				point = point * 25.4 / dpi;
				return point;
			}

			assert(false);
			return 0.0;
		}

		if (convertFrom == VFrame30::SchemaUnit::Inch)
		{
			if (convertTo == VFrame30::SchemaUnit::Display)
			{
				if (dpi == 0)
				{
					assert(dpi != 0);
					return 0.0;
				}

				point = point * dpi;
				return point;
			}

			if (convertTo == VFrame30::SchemaUnit::Millimeter)
			{
				point = point * 25.4;
				return point;
			}

			assert(false);
			return 0.0;
		}

		if (convertFrom == VFrame30::SchemaUnit::Millimeter)
		{
			if (convertTo == VFrame30::SchemaUnit::Display)
			{
				if (dpi == 0)
				{
					assert(dpi != 0);
					return 0.0;
				}

				point = point * dpi / 25.4;
				return point;
			}

			if (convertTo == VFrame30::SchemaUnit::Inch)
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

	static void ConvertPoint(double& x, double& y, const VFrame30::SchemaUnit convertFrom, const VFrame30::SchemaUnit convertTo, const int dpiX, const int dpiY)
	{
		if (convertFrom == convertTo)
		{
			return;
		}

		if (convertFrom == VFrame30::SchemaUnit::Display)
		{
			if (convertTo == VFrame30::SchemaUnit::Inch)
			{
				if (dpiX == 0 || dpiY == 0)
				{
					assert(dpiX != 0);
					assert(dpiY != 0);
					return;
				}

				x = x / dpiX;
				y = y / dpiY;
				return;
			}

			if (convertTo == VFrame30::SchemaUnit::Millimeter)
			{
				if (dpiX == 0 || dpiY == 0)
				{
					assert(dpiX != 0);
					assert(dpiY != 0);
					return;
				}

				x = x * 25.4 / dpiX;
				y = y * 25.4 / dpiY;
				return;
			}

			assert(false);
			return;
		}

		if (convertFrom == VFrame30::SchemaUnit::Inch)
		{
			if (convertTo == VFrame30::SchemaUnit::Display)
			{
				if (dpiX == 0 || dpiY == 0)
				{
					assert(dpiX != 0);
					assert(dpiY != 0);
					return;
				}

				x = x * dpiX;
				y = y * dpiY;
				return;
			}

			if (convertTo == VFrame30::SchemaUnit::Millimeter)
			{
				x = x * 25.4;
				y = y * 25.4;
				return;
			}

			assert(false);
			return;
		}

		if (convertFrom == VFrame30::SchemaUnit::Millimeter)
		{
			if (convertTo == VFrame30::SchemaUnit::Display)
			{
				if (dpiX == 0 || dpiY == 0)
				{
					assert(dpiX != 0);
					assert(dpiY != 0);
					return;
				}

				x = x * dpiX / 25.4;
				y = y * dpiY / 25.4;
				return;
			}

			if (convertTo == VFrame30::SchemaUnit::Inch)
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

	static QPointF snapToGrid(QPointF pt, double gridSize)
	{
		QPointF result;

		// SnapToGrid для Xin
		//
		double restX = pt.x() - floor(pt.x() / gridSize) * gridSize;

		if (restX <= gridSize / 2)
		{
			result.setX(floor(pt.x() / gridSize) * gridSize);
		}
		else
		{
			result.setX(floor(pt.x() / gridSize) * gridSize + gridSize);
		}

		// SnapToGrid для YXin
		//
		double restY = pt.y() - floor(pt.y() / gridSize) * gridSize;

		if (restY <= gridSize / 2)
		{
			result.setY(floor(pt.y() / gridSize) * gridSize);
		}
		else
		{
			result.setY(floor(pt.y() / gridSize) * gridSize + gridSize);
		}

		return result;
	}


	static QPointF snapToGrid(double x, double y, double gridSize)
	{
		QPointF result;

		// SnapToGrid для Xin
		//
		double restX = x - floor(x / gridSize) * gridSize;

		if (restX <= gridSize / 2)
		{
			result.setX(floor(x / gridSize) * gridSize);
		}
		else
		{
			result.setX(floor(x / gridSize) * gridSize + gridSize);
		}

		// SnapToGrid для YXin
		//
		double restY = y - floor(y / gridSize) * gridSize;

		if (restY <= gridSize / 2)
		{
			result.setY(floor(y / gridSize) * gridSize);
		}
		else
		{
			result.setY(floor(y / gridSize) * gridSize + gridSize);
		}

		return result;
	}

	static double snapToGrid(double value, double gridSize)
	{
		double rest = value - floor(value / gridSize) * gridSize;

		if (rest <= gridSize / 2)
		{
			return std::floor(value / gridSize) * gridSize;
		}
		else
		{
			return std::floor(value / gridSize) * gridSize + gridSize;
		}
	}

	/// <summary>
	/// Опредение пересечения двух отрезков
	/// </summary>
	/// <returns>true - отрезки пересекаются</returns>
	static bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2)
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
	static bool IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle)
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


	/// <summary>
	/// Вычисление хэш кода имени класса
	/// </summary>
	/// <remarks>
	/// Сохраняется в сериализуемом файле, не изменять способ вычисления
	/// </remarks>
	static quint32 GetClassHashCode(const std::string& className)
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
		{
			nHash += (nHash << 5) + *ptr++;
		}

		return nHash;
	}


	static bool processDiagSignalMask(const QString& mask, const QString& str)
	{
		if (mask.isEmpty())
		{
			return true;
		}

		int maskLen = mask.length();
		int strLen = str.length();

		int maskPos = 0;	// текущая позиция маски
		int strPos = 0;		// текущая позиция строки

		while(true)
		{
			if (strPos == strLen && maskPos == maskLen)		// дошли до конца строки и маски строки совпали
			{
				return true;
			}

			if (strPos == strLen)		// дошли до конца строки строки не совпали
			{
				return false;
			}

			if (maskPos == maskLen)		// дошли до конца маски - строки не совпали
			{
				return false;
			}

			QChar m = mask.at(maskPos);

			if (m == '~')				// если в маске встретили тильду - вернуть Т� У
			{
				return true;
			}

			if (m == '?')				// ? - пропустить символ и начать сначала
			{
				maskPos++;
				strPos++;
				continue;
			}

			if (m == '*')				//* - пропустить все до первого символа "_"
			{
				while (str.at(strPos) != '_')
				{
					strPos++;

					if (strPos == strLen)		// дошли до конца строки - строки совпали
					{
						if (maskPos == maskLen - 1)
						{
							return true;		//строки совпали если это последняя звездочка
						}
						else
						{
							return false;
						}
					}
				}

				maskPos++;
				continue;
			}

			if (str.at(strPos) != m)			// очередные символы не совпали
			{
				return false;
			}

			strPos++;
			maskPos++;
		}

		return true;
	}

    static quint16 calcHash16(const void* src, int l)
    {
        if (src == nullptr)
        {
            assert(src);
            return 0;
        }

        quint16 nHash = 0;
		quint8* p = (quint8*)src;

        while (l--)
		{
            nHash += (nHash<<5) + *p++;
		}

        return nHash;
    }

	static quint64 calcHash(const void* src, qint64 l)
	{
		if (src == nullptr)
		{
			assert(src);
			return UNDEFINED_HASH;
		}

		quint64 nHash = 0;
		quint8* p = (quint8*)src;

		while (l--)
		{
			nHash += (nHash<<5) + *p++;
		}

		return nHash;

	}

	static QString dateTimeToStringTime(const QDateTime& dt, bool milliseconds)
	{
		QTime tm = dt.time();
		QString result;

		if (milliseconds == true)
		{
			result = QString("%1:%2:%3.%4")
						.arg(tm.hour(), 2, 10, QChar('0'))
						.arg(tm.minute(), 2, 10, QChar('0'))
						.arg(tm.second(), 2, 10, QChar('0'))
						.arg(tm.msec(), 3, 10, QChar('0'));
		}
		else
		{
			result = QString("%1:%2:%3")
						.arg(tm.hour(), 2, 10, QChar('0'))
						.arg(tm.minute(), 2, 10, QChar('0'))
						.arg(tm.second(), 2, 10, QChar('0'));
		}

		return result;
	}

	static QString dateTimeToStringDate(const QDateTime& dt)
	{
		QDate date = dt.date();
		QString result;

		result = QString("%1.%2.%3")
					.arg(date.day(), 2, 10, QChar('0'))
					.arg(date.month(), 2, 10, QChar('0'))
					.arg(date.year(), 4, 10, QChar('0'));

		return result;
	}
};



