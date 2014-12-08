#pragma once

#include "TypesAndEnums.h"

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

// ��������� ���������� � �����
//
#define mm2in(mmVal) (mmVal / 25.4)

// ��������� ����� � ����������
//
#define in2mm(inVal) (inVal * 25.4)

// ������ ��� ����������� �������
//
#define ASSERT_IF(expr) std::assert(!(expr))

class CUtils
{
public:
	// ���������� ������ ����� ������� ��� ���������� ��� ������
	static const int InchesRoundDigits = 4;
	// ���������� ������ ����� ������� ��� ���������� ��� �����������
	static const int MillimetresRoundDigits = 2;

	// �������� DpiX ��� �������, ���������������� �� ������ ������
	static int DpiX()
	{
		static int dpiX = -1;
		if (dpiX == -1)
		{
			dpiX = QApplication::desktop()->screen()->logicalDpiX();
		}
		return dpiX;
	}

	// �������� DpiY ��� �������
	static int DpiY()
	{
		static int dpiY = -1;
		if (dpiY == -1)
		{
			dpiY = QApplication::desktop()->screen()->logicalDpiY();
		}
		return dpiY;
	}

	// Math functions
	//

	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	// mode ���������, ��������� ������������� ������� �����, ��� AwayFromZero, �.�. 1.5 -> 2.0
	//
	static double Round(double value, int digits, VFrame30::MidpointRounding mode)	// ����������, ����� �� ���������� ���������� ������ ������� �����.
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

	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	static double Round(double value, int digits)
	{
		// ����������, ����� �� ���������� ���������� ������ ������� �����.
		// 2.224 (2 ��.) -> 2.22, 2.5352 (2 ��) -> 2.54, 3.8 (0 ��.) -> 4
		//
		// value - �������� ��� ����������
		// digits - ���������� ����� ������� ����� � ������������ ��������
		//
		double intpart = 0.0;
		double fractpart = modf(value, &intpart);

		if(digits != 0)
		{
			int bal = 0;		// ������� �� �������
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

	// ����������, �� ������.
	static double Round(double value)
	{
		// ����������, �� ������.
		// 2.2 -> 2, 2.5 -> 3, 3.8 -> 4
		//

		// value - �������� ��� ����������
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

	// �� ���� floor, ������������ ��� ���������� ��������� ��� SchemeUnit::Display
	static double RoundDisplayPoint(double value)
	{
		return floor(value);
	}

	// ��������� �������� � ����������� �� ���� ������������ �������
	static double RoundPoint(double p, VFrame30::SchemeUnit unit)
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

	static double ConvertPoint(double point, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo, VFrame30::ConvertDirection convertDirection)
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

	static void ConvertPoint(double& x, double& y, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo)
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

	static QPointF snapToGrid(const QPointF& pt, double gridSize)
	{
		QPointF result;

		// SnapToGrid ��� Xin
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

		// SnapToGrid ��� YXin
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

		// SnapToGrid ��� Xin
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

		// SnapToGrid ��� YXin
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
			return floor(value / gridSize) * gridSize;
		}
		else
		{
			return floor(value / gridSize) * gridSize + gridSize;
		}
	}

	/// <summary>
	/// ��������� ����������� ���� ��������
	/// </summary>
	/// <returns>true - ������� ������������</returns>
	static bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2)
	{
		double v1 = (bx2 - bx1) * (ay1 - by1) - (by2 - by1) * (ax1 - bx1);
		double v2 = (bx2 - bx1) * (ay2 - by1) - (by2 - by1) * (ax2 - bx1);
		double v3 = (ax2 - ax1) * (by1 - ay1) - (ay2 - ay1) * (bx1 - ax1);
		double v4 = (ax2 - ax1) * (by2 - ay1) - (ay2 - ay1) * (bx2 - ax1);
		return (v1 * v2 < 0) && (v3 * v4 < 0);
	}


	/// <summary>
	/// �����������, ���������� �� ����� ��������� �������������
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
	/// ���������� ��� ���� ����� ������
	/// </summary>
	/// <remarks>
	/// ����������� � ������������� �����, �� �������� ������ ����������
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
			nHash += (nHash<<5) + *ptr++;

		return nHash;
	}

};



