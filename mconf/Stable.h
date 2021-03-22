// C++/C includes
//
#include <assert.h>
#include <stdint.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
#endif

#include <QtCore>
#include <QtWidgets>
#include <QDateEdit>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QtNetwork/QHostInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QtEndian>

#include <QSerialPort>
#include <QSerialPortInfo>

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

// Log
//
#include "../lib/OutputLog.h"

extern OutputLog theLog;
