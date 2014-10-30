#pragma once

#include "TypesAndEnums.h"

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
	CUtils(void);
	~CUtils(void);

	// --
	//
	static const int InchesRoundDigits = 4;			// ���������� ������ ����� ������� ��� ���������� ��� ������
	static const int MillimetresRoundDigits = 2;	// ���������� ������ ����� ������� ��� ���������� ��� �����������

	static int DpiX();								// �������� DpiX ��� �������, ���������������� �� ������ ������
	static int DpiY();								// �������� DpiY ��� �������

	// Math functions
	//

	static double Round(double value, int digits, VFrame30::MidpointRounding mode);	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	static double Round(double value, int digits);	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	static double Round(double value);				// ����������, �� ������.

	static double RoundDisplayPoint(double value);			// �� ���� floor, ������������ ��� ���������� ��������� ��� SchemeUnit::Display
	static double RoundPoint(double p, VFrame30::SchemeUnit unit);	// ��������� �������� � ����������� �� ���� ������������ �������

	static double ConvertPoint(double point, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo, VFrame30::ConvertDirection convertDirection);
	static void ConvertPoint(double& x, double& y, VFrame30::SchemeUnit convertFrom, VFrame30::SchemeUnit convertTo);

	static QPointF snapToGrid(const QPointF& pt, double gridSize);
	static QPointF snapToGrid(double x, double y, double gridSize);
	static double snapToGrid(double value, double gridSize);

	/// <summary>
	/// ��������� ����������� ���� ��������
	/// </summary>
	/// <returns>true - ������� ������������</returns>
	static bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2);

	/// <summary>
	/// �����������, ���������� �� ����� ��������� �������������
	/// </summary>
	static bool IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle);

	/// <summary>
	/// ���������� ��� ���� ����� ������
	/// </summary>
	/// <remarks>
	/// ����������� � ������������� �����, �� �������� ������ ����������
	/// </remarks>
	static quint32 GetClassHashCode(const std::string& className);

};



