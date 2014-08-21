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

namespace VFrame30
{
	class CVFrameUtils
	{
	public:
		CVFrameUtils(void);
		~CVFrameUtils(void);

		// --
		//
		static const int InchesRoundDigits = 4;			// ���������� ������ ����� ������� ��� ���������� ��� ������
		static const int MillimetresRoundDigits = 2;	// ���������� ������ ����� ������� ��� ���������� ��� �����������

		static int DpiX();								// �������� DpiX ��� �������, ���������������� �� ������ ������
		static int DpiY();								// �������� DpiY ��� �������	
		
		// Math functions
		//
		
		static double Round(double value, int digits, MidpointRounding mode);	// ����������, ����� �� ���������� ���������� ������ ������� �����.
		static double Round(double value, int digits);	// ����������, ����� �� ���������� ���������� ������ ������� �����. 
		static double Round(double value);				// ����������, �� ������. 
		
		static double RoundDisplayPoint(double value);			// �� ���� floor, ������������ ��� ���������� ��������� ��� SchemeUnit::Display
		static double RoundPoint(double p, SchemeUnit unit);	// ��������� �������� � ����������� �� ���� ������������ ������� 

		static double ConvertPoint(double point, SchemeUnit convertFrom, SchemeUnit convertTo, ConvertDirection convertDirection);
		static void ConvertPoint(double& x, double& y, SchemeUnit convertFrom, SchemeUnit convertTo);

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
}



