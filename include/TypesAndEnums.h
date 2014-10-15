#pragma once

namespace VFrame30
{
	// SchemeUnit
	//
	enum class SchemeUnit
	{
		Display,			// ��������� � ������ ���������� (������� ��� �������)
		Millimeter,			// ��������� � ��
		Inch				// ��������� � ������
	};

	// ����������� ��������������� ���������, �� ����� ���� ������������ ����� Dpi (DpiX/DpiY) ������������
	//
	enum ConvertDirection
	{
		Horz,
		Vert
	};

	// ��� ����������
	//
	enum MidpointRounding
	{
		ToFloor,			// Floor rounding
		AwayFromZero		// When a number is halfway between two others, it is rounded toward the nearest number that is away from zero.
	};
}

