#pragma once

#include <memory>
#include <map>
#include <unordered_map>
#include <optional>
#include <queue>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 6326)		// MSVC warning C6326: potential comparison of a constant with another constant
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
	#pragma warning(disable : 6386)		// Static analyzer warning: qvariant.h:444: warning: C6386: Buffer overrun while writing to 'data':  the writable size is 'size' bytes, but '8' bytes might be written.: Lines: 431, 432, 433, 435, 436, 443, 444
	#pragma warning(disable : 6385)		// Static analyzer warning: qhash.h:367: warning: C6385: Reading invalid data from 'this->offsets':  the readable size is '128' bytes, but 'i' bytes may be read.: Lines: 366, 367
#endif

#include <QtCore>
#include <QtQml>
#include <QtNetwork>
#include <QtConcurrent>
#include <QtXml>
#include <QVector>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


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
