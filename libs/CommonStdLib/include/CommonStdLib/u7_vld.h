#pragma once

// Visual Leak Detector
//
#if defined(_MSC_VER) && defined(_DEBUG)
	#if __has_include("C:/Program Files (x86)/Visual Leak Detector/include/vld.h")
		#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
		#define VLD_IS_INCLUDED
	#endif
#endif // Visual Leak Detector

namespace Vld
{
	void setVldReportFilterHook();
}
