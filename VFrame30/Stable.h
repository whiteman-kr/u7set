#ifndef STABLE_H
#define STABLE_H

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
#include <unordered_map>
#include <set>
#include <cmath>
#include <cassert>
#include <memory>
#include <utility>
#include <string>
#include <algorithm>
#include <functional>
#include <iterator>
#include <type_traits>
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
#include <qglobal.h>
#include <QApplication>
#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QPushButton>
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStaticText>
#include <QScreen>
#include <QUuid>
#include <QDebug>
#include <QRect>
#include <QMutex>
#include <QJSEngine>
#include <QQmlEngine>
#include <QAction>
#include <QPdfWriter>

#include <QXmlStreamReader>
#include <QDomElement>

#include <QJsonDocument>
#include <QJsonObject>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Project stable includes
//
#include "VFrame30Lib_global.h"
#include "../lib/Factory.h"
#include "../lib/TypesAndEnums.h"
#include "../lib/Types.h"
#include "../lib/CUtils.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/DebugInstCounter.h"

#ifdef _MSC_VER
	#pragma warning(disable : 4482)
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

#endif // STABLE_H
