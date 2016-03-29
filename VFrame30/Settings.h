#pragma once

#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT Settings
	{
		Settings(void);
		~Settings(void);

	public:
		static SchemaUnit regionalUnit(void);
		static void setRegionalUnit(SchemaUnit value);

		static double defaultGridSize(SchemaUnit unit);
		
	private:
		static SchemaUnit m_regionalUnit;

		// ћинимальный грид, дл€ схем, используетс€ дл€ позиционировани€ (выравнивание€) пинов в Fbl едементах
		//
		static const double m_defaultGridSizeIn;
		static const double m_defaultGridSizeMm;
		static const double m_defaultGridSizePx;
	};
}


