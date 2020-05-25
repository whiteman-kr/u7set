#pragma once

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>


// C++ includes
//
#include <array>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <limits>
#include <iterator>
#include <optional>


// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 6326)		// MSVC warning C6326: potential comparison of a constant with another constant
#endif

#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QtXmlPatterns>
#include <QtSvg>
#include <QtSerialPort>
#include <QtDebug>
#include <QtQml>
#include <QtNetwork>
#include <QtSql>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


// Other stable includes
//
#include "../VFrame30/VFrame30Lib_global.h"
#include "../lib/Factory.h"
#include "../lib/TypesAndEnums.h"
#include "../lib/Types.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/DebugInstCounter.h"
#include "../lib/CUtils.h"

// Disable some warnings
//
#ifdef _MSC_VER
	#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name
	#pragma warning(disable : 4251)		// Static analyzer warning: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	#pragma warning(disable : 4275)		// Static analyzer warning: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'

		// Disable 4996 warning
	#ifndef _SCL_SECURE_NO_WARNINGS
		#define _SCL_SECURE_NO_WARNINGS
	#endif
#endif


// For detecting memory leaks
//
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif

