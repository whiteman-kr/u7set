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

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
#endif

#include <QAbstractSocket>
#include <QApplication>

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

	// Disable 4996 warning
#ifndef _SCL_SECURE_NO_WARNINGS
	#define _SCL_SECURE_NO_WARNINGS
#endif
#endif

// Turn on memory leaks detection
//

#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif
#endif


