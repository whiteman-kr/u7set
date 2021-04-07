#pragma once

#include "../lib/TypesAndEnums.h"

namespace VFrame30
{
	class Settings
	{
		Settings(void);
		~Settings(void);

	public:
		static SchemaUnit regionalUnit(void);
		static void setRegionalUnit(SchemaUnit value);

		static double defaultGridSize(SchemaUnit unit);
		
	private:
		static SchemaUnit m_regionalUnit;

		// ����������� ����, ��� ����, ������������ ��� ���������������� (�������������) ����� � Fbl ���������
		//
		static const double m_defaultGridSizeIn;
		static const double m_defaultGridSizeMm;
		static const double m_defaultGridSizePx;
	};
}


