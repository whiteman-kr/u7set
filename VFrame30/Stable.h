#ifndef STFAFX_H
#define STFAFX_H

// VFrame30 Library
//
#ifdef _MSC_VER
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4481)
	#pragma warning(disable : 4482)
#endif

// Standard Template Library includes 
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4996)		// You will see C4996 if you are using members of the <hash_map> and <hash_set> header files in the std namespace.
#endif
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <cmath>
#include <assert.h>
#include <memory>
#include <string>
#include <algorithm>
#include <iterator>
#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 4800)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4512)
#endif
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QScreen>
#include <QUuid>
#include <QDebug>
#include <QRect>
#include <QMutex>
#include <QtScript/QtScript>
#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Project stable includes
//
#include "VFrame30Lib_global.h"
#include "Factory.h"
#include "../include/TypesAndEnums.h"
#include "../include/VFrameUtils.h"

#ifdef _MSC_VER
	#pragma warning(disable : 4482)
#endif

#endif	// STFAFX_H
