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

		static double minFblGridSize(SchemeUnit unit);
		
	private:
		static SchemeUnit m_regionalUnit;

		// ћинимальный грид, дл€ схем, используетс€ дл€ позиционировани€ (выравнивание€) пинов в Fbl едементах
		//
		static const double m_minFblGridSizeIn;
		static const double m_minFblGridSizeMm;
		static const double m_minFblGridSizePx;
	};
}


