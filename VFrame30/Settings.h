#pragma once

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

		// Minimum grid for schemas. Used for positioning pins in FBL items
		//
		static const double m_defaultGridSizeIn;
		static const double m_defaultGridSizeMm;
		static const double m_defaultGridSizePx;
	};
}


