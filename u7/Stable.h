#pragma once

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>


// C++ includes
//
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


// Qt includes
//
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
#ifdef Q_OS_WIN
#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name

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

