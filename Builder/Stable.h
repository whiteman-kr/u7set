#ifndef STABLE_H
#define STABLE_H

#include <cassert>
#include <typeindex>
#include <functional>
#include <set>
#include <memory>

#include <QtCore>
#include <QtGui>
#include <QtQml>
#include <QtConcurrent>

#include "../VFrame30/VFrame30Lib_global.h"

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

#endif // STABLE_H
