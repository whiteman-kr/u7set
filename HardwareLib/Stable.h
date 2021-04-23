#pragma once

// C includes, must be before c++ includes
//
#include <cassert>
#include <cstdint>

// C++ includes
//
#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <type_traits>
#include <utility>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
#endif


#include <QtCore>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


#define HARDWARE_LIB_DOMAIN

