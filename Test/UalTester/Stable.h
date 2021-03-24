#pragma once

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>

// C++ includes
//
#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <type_traits>

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
	#pragma warning(disable : 28182)
#endif

// Qt includes
//
#include <QAbstractSocket>

#include <QCommandLineParser>
#include <QCoreApplication>

#include <QDebug>
#include <QDateTime>

#include <QElapsedTimer>
#include <QEventLoop>

#include <QFile>
#include <QFileInfo>

#include <QHash>
#include <QHostAddress>

#include <QMap>
#include <QMetaClassInfo>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>

#include <QObject>

#include <QQueue>

#include <QSettings>
#include <QString>
#include <QStringList>

#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QTranslator>

#include <QtCore>
#include <QtGlobal>
#include <QtNetwork/QHostAddress>

#include <QUdpSocket>
#include <QUuid>

#include <QVariant>
#include <QVector>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

// Disable some warnings
//
#ifdef Q_OS_WIN
	#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name
	#pragma warning(disable : 4251)

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


