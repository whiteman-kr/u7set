#pragma once

// Add C includes here
//

// Add C++ includes here
//
#include <cassert>
#include <vector>
#include <array>
#include <map>
#include <list>
#include <memory>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <cfloat>

// Add Qt includes here
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)		// MSVC warning
	#pragma warning(disable : 6326)		// MSVC warning C6326: potential comparison of a constant with another constant
	#pragma warning(disable : 4251)		// MSVC warning
	#pragma warning(disable : 4127)
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
#endif

#include <QtCore>
#include <QtWidgets>
#include <QPaintEngine>
#include <QPainter>
#include <QtPrintSupport>
#include <QWidget>
#include <QPdfWriter>
#include <QMouseEvent>
#include <QColorDialog>
#include <QVector>
#include <QHash>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Add thirdparty and any stable includes here
//
//#include "thirdparty/include/libmain.h"
#include "../lib/TimeStamp.h"

// Visual Leak Detector
//
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
#if __has_include("C:/Program Files (x86)/Visual Leak Detector/include/vld.h")
	#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#else
	#if __has_include("D:/Program Files (x86)/Visual Leak Detector/include/vld.h")
		#include "D:/Program Files (x86)/Visual Leak Detector/include/vld.h"
	#endif
#endif
#endif	// Visual Leak Detector

