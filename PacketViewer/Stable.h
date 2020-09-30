#ifndef STABLE_H
#define STABLE_H

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4251)
#endif
// C++ includes
//
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <limits>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
	#pragma warning(disable : 4251)
#endif

// Qt includes
//
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QtConcurrent>
#include <QtQml>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Disable some warnings
//
#ifdef Q_OS_WIN
#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name

	// Disable 4996 warning
#ifndef _SCL_SECURE_NO_WARNINGS
	#define _SCL_SECURE_NO_WARNINGS
#endif
#endif

#endif // STABLE_H

