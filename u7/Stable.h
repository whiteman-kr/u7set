#pragma once

// For detecting memory leaks
//
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#ifdef _DEBUG
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	//#define DBG_NEW new
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#else
	 #define new DBG_NEW
#endif
#endif

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
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
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

#ifdef Q_OS_WINDOWS
	#include <QtWinExtras>
#endif

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


// Other stable includes
//
#include "../CommonLib/Factory.h"
#include "../CommonLib/Types.h"
#include "../CommonLib/DebugInstCounter.h"
#include "../Proto/ProtoSerialization.h"

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

