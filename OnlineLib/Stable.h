#pragma once

// C includes, must be before c++ includes
//
#include <cassert>
#include <cstdint>

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
	#pragma warning(disable : 28182)	// C:\Qt\5.15.0\msvc2019_64\include\QtCore\qvector.h(761) : warning C28182: Dereferencing NULL pointer. 'd' contains the same NULL value as 'x' did. See line 713 for an earlier location where this can occur: Lines: 702, 703, 705, 709, 710, 713, 715, 716, 718, 719, 720, 722, 724, 729, 746, 748, 749, 758, 760, 761
	#pragma warning(disable : 6386)		// Static analyzer warning: qvariant.h:444: warning: C6386: Buffer overrun while writing to 'data':  the writable size is 'size' bytes, but '8' bytes might be written.: Lines: 431, 432, 433, 435, 436, 443, 444
	#pragma warning(disable : 6385)		// Static analyzer warning: qhash.h:367: warning: C6385: Reading invalid data from 'this->offsets':  the readable size is '128' bytes, but 'i' bytes may be read.: Lines: 366, 367
#endif

#include <QAbstractSocket>

#include <QCommandLineParser>

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

#define ONLINE_LIB_DOMAIN
