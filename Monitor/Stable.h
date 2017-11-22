#ifndef MONITOR_STABLE_H
#define MONITOR_STABLE_H

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>

// C++ includes
//
#include <memory>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <limits>

// Qt includes
//
#include <QtCore>
#include <QtWidgets>
#include <QtXml>

#include <QTimeEdit>
#include <QDateEdit>

// Other stable includes
//
#include "../VFrame30/VFrame30Lib_global.h"
//#include "../lib/Factory.h"
//#include "../lib/TypesAndEnums.h"
//#include "../lib/CUtils.h"

#include "../lib/Types.h"

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
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
   #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
   #define new DBG_NEW
#endif
#endif


#include "../lib/AppSignalManager.h"

extern AppSignalManager theSignals;


#endif // MONITOR_STABLE_H

