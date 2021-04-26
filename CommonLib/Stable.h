#pragma once

// C includes, must be before c++ includes
//
#include <cassert>
#include <cstdint>

// C++ includes
//
//#include <algorithm>
//#include <atomic>
//#include <functional>
//#include <iostream>
//#include <limits>
//#include <list>
//#include <map>
//#include <memory>
//#include <vector>
//#include <set>
//#include <type_traits>
//#include <utility>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
	#pragma warning(disable : 28182)
#endif

#include <QVector>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


#define COMMON_LIB_DOMAIN

