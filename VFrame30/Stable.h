#pragma once

// VFrame30 Library
//
#ifdef _MSC_VER
	#pragma warning(disable : 4251)
//	#pragma warning(disable : 4481)
//	#pragma warning(disable : 4482)
#endif

// Standard Template Library includes 
//
//#ifdef _MSC_VER
//	#pragma warning(push)
//	#pragma warning(disable : 6011)
//	#pragma warning(disable : 4996)		// You will see C4996 if you are using members of the <hash_map> and <hash_set> header files in the std namespace.
//#endif

#include <array>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cmath>
#include <cassert>
#include <memory>
#include <utility>
#include <string>
#include <optional>
#include <variant>
#include <algorithm>
#include <functional>
#include <optional>
#include <iterator>
#include <type_traits>
//#ifdef _MSC_VER
//	#pragma warning(pop)
//#endif

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4512)
	#pragma warning(disable : 4800)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 6326)		// MSVC warning C6326: potential comparison of a constant with another constant
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
#endif

#include <qglobal.h>
#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
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
#include <QDrag>
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
#include "../lib/Factory.h"
#include "../CommonLib/Types.h"
#include "../lib/CUtils.h"
#include "../lib/DebugInstCounter.h"
#include "../CommonLib/PropertyObject.h"
#include "../Proto/ProtoSerialization.h"

#ifdef _MSC_VER
	#pragma warning(disable : 4482)
#endif

