#include "Stable.h"
#include <assert.h>
#include "../include/VFrameUtils.h"

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4800)
#endif
#include <QtWidgets/QDesktopWidget>
#ifdef _MSC_VER
	#pragma warning(pop)
#endif

namespace VFrame30
{

	CVFrameUtils::CVFrameUtils(void)
	{
	}


	CVFrameUtils::~CVFrameUtils(void)
	{
	}

	int CVFrameUtils::DpiX()
	{
		static int dpiX = -1;
		if (dpiX == -1)
		{
			dpiX = QApplication::desktop()->screen()->logicalDpiX();
		}
		return dpiX;
	}

	int CVFrameUtils::DpiY()
	{
		static int dpiY = -1;
		if (dpiY == -1)
		{
			dpiY = QApplication::desktop()->screen()->logicalDpiY();
		}
		return dpiY;
	}


	// ����������, ����� �� ���������� ���������� ������ ������� �����. 
	// mode ���������, ��������� ������������� ������� �����, ��� AwayFromZero, �.�. 1.5 -> 2.0
	//
	double CVFrameUtils::Round(double value, int digits, MidpointRounding mode)
	{
		if (mode == MidpointRounding::ToFloor)
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
	// 2.224 (2 ��.) -> 2.22, 2.5352 (2 ��) -> 2.54, 3.8 (0 ��.) -> 4
	//
	double CVFrameUtils::Round(double value, int digits)
	{
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
	// 2.2 -> 2, 2.5 -> 3, 3.8 -> 4
	//
	double CVFrameUtils::Round(double value)
	{
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

	double CVFrameUtils::RoundDisplayPoint(double value)	// �� ���� floor, ������������ ��� ���������� ��������� ��� SchemeUnit::Display
	{
		return floor(value);
	}

	double CVFrameUtils::RoundPoint(double p, SchemeUnit unit)				// ��������� �������� � ����������� �� ���� ������������ ������� 
	{
		switch (unit)
		{ 
		case SchemeUnit::Display:
			assert(false);
			return 0;

		case SchemeUnit::Inch:
			return Round(p, InchesRoundDigits, MidpointRounding::AwayFromZero);

		case SchemeUnit::Millimeter:
			return Round(p, MillimetresRoundDigits, MidpointRounding::AwayFromZero);

		default:
			assert(false);
			return 0;
		}
	}

	double CVFrameUtils::ConvertPoint(double point, SchemeUnit convertFrom, SchemeUnit convertTo, ConvertDirection convertDirection)
	{
		if (convertFrom == convertTo)
		{
			return point;
		}

		int dpi = convertDirection == ConvertDirection::Horz ? DpiX() : DpiY();
		if (dpi == 0)
		{
			assert(dpi != 0);
			return 0.0;
		}

		if (convertFrom == SchemeUnit::Display)
		{
			if (convertTo == SchemeUnit::Inch)
			{
				point = point / dpi;
				return point;
			}

			if (convertTo == SchemeUnit::Millimeter)
			{
				point = point * 25.4 / dpi;
				return point;
			}

			assert(false);
			return 0.0;
		}

		if (convertFrom == SchemeUnit::Inch)
		{
			if (convertTo == SchemeUnit::Display)
			{
				point = point * dpi;
				return point;
			}

			if (convertTo == SchemeUnit::Millimeter)
			{
				point = point * 25.4;
				return point;
			}

			assert(false);
			return 0.0;
		}

		if (convertFrom == SchemeUnit::Millimeter)
		{
			if (convertTo == SchemeUnit::Display)
			{
				point = point * dpi / 25.4;
				return point;
			}

			if (convertTo == SchemeUnit::Inch)
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

	void CVFrameUtils::ConvertPoint(double& x, double& y, SchemeUnit convertFrom, SchemeUnit convertTo) 
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

		if (convertFrom == SchemeUnit::Display)
		{
			if (convertTo == SchemeUnit::Inch)
			{
				x = x / dpiX;
				y = y / dpiY;
				return;
			}

			if (convertTo == SchemeUnit::Millimeter)
			{
				x = x * 25.4 / dpiX;
				y = y * 25.4 / dpiY;
				return;
			}

			assert(false);
			return;
		}

		if (convertFrom == SchemeUnit::Inch)
		{
			if (convertTo == SchemeUnit::Display)
			{
				x = x * dpiX;
				y = y * dpiY;
				return;
			}

			if (convertTo == SchemeUnit::Millimeter)
			{
				x = x * 25.4;
				y = y * 25.4;
				return;
			}

			assert(false);
			return;
		}

		if (convertFrom == SchemeUnit::Millimeter)
		{
			if (convertTo == SchemeUnit::Display)
			{
				x = x * dpiX / 25.4;
				y = y * dpiY / 25.4;
				return;
			}

			if (convertTo == SchemeUnit::Inch)
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
	/// ��������� ����������� ���� ��������
	/// </summary>
	/// <returns>true - ������� ������������</returns>
	bool CVFrameUtils::IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2)
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
	bool CVFrameUtils::IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle)
	{
		if (intersectRectangle.contains(ax1, ay1) == true ||
			intersectRectangle.contains(ax2, ay2) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			intersectRectangle.x(), intersectRectangle.y(),
			intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y(),
			intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y() + intersectRectangle.height()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			intersectRectangle.x() + intersectRectangle.width(), intersectRectangle.y() + intersectRectangle.height(),
			intersectRectangle.x(), intersectRectangle.y() + intersectRectangle.height()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			intersectRectangle.x(), intersectRectangle.y() + intersectRectangle.height(),
			intersectRectangle.x(), intersectRectangle.y()) == true)
		{
			return true;
		}

		return false;
	}

	// ���������� ��� ���� ����� ������, ����������� � ������������� �����, �� �������� ������ ����������, 
	// ������ ��� ��������� � ����.
	// ��� ������ ����� ���������� namespace, �� �� ����� ������� 
	// �.�. VFrame30::CVideoFrame ����� ��������� �� CVideoFrame
	//
	quint32 CVFrameUtils::GetClassHashCode(const std::string& className)
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

}
