#ifndef STABLE_H
#define STABLE_H

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

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Add thirdparty and any stable includes here
//
//#include "thirdparty/include/libmain.h"
#include "../lib/TimeStamp.h"

#endif // STABLE_H
