#pragma once

#include "VFrame30Lib_global.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT VFrame30Library
	{
	public:
		VFrame30Library();
		
		// Initialize VFrame30 Library, must be called before using any library classes
		//
		static bool Init();
		static bool Shutdown();
	};
}


