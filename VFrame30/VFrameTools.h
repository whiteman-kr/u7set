#pragma once
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

	// ����������, �� ������.
	//
	double Round(double value);

	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	//
	double Round(double value, int digits);

	// ����������, ����� �� ���������� ���������� ������ ������� �����.
	// mode ���������, ��������� ������������� ������� �����, ��� AwayFromZero, �.�. 1.5 -> 2.0
	//
	double Round(double value, int digits, MidpointRounding mode);

	// �� ���� floor, ������������ ��� ���������� ��������� ��� SchemaUnit::Display
	//
	double RoundDisplayPoint(double value);

	// ��������� �������� � ����������� �� ���� ������������ �������
	//
	double RoundPoint(double p, SchemaUnit unit);

	//
	// Convert functions
	//
	extern constexpr double mm2in(double mmVal);
	extern constexpr double in2mm(double inVal);

	double ConvertPoint(double point, SchemaUnit convertFrom, SchemaUnit convertTo, int dpi);
	void ConvertPoint(double& x, double& y, const SchemaUnit convertFrom, const SchemaUnit convertTo, const int dpiX, const int dpiY);

	QPointF snapToGrid(QPointF pt, double gridSize);
	QPointF snapToGrid(double x, double y, double gridSize);
	double snapToGrid(const double value, const double gridSize);

	/// <summary>
	/// ��������� ����������� ���� ��������
	/// </summary>
	/// <returns>true - ������� ������������</returns>
	bool IsLineIntersected(double ax1, double ay1, double ax2, double ay2, double bx1, double by1, double bx2, double by2);

	/// <summary>
	/// �����������, ���������� �� ����� ��������� �������������
	/// </summary>
	bool IsLineIntersectRect(double ax1, double ay1, double ax2, double ay2, const QRectF& intersectRectangle);
}
