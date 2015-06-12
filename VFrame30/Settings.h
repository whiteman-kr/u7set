#pragma once

#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT Settings
	{
		Settings(void);
		~Settings(void);

	public:
		static SchemeUnit regionalUnit(void);
		static void setRegionalUnit(SchemeUnit value);

		static double defaultGridSize(SchemeUnit unit);
		
	private:
		static SchemeUnit m_regionalUnit;

		// ����������� ����, ��� ����, ������������ ��� ���������������� (�������������) ����� � Fbl ���������
		//
		static const double m_defaultGridSizeIn;
		static const double m_defaultGridSizeMm;
		static const double m_defaultGridSizePx;
	};
}


