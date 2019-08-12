#ifndef STABLE_H
#define STABLE_H

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>


// C++ includes
//
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <limits>


// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
#endif

#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QtNetwork/QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QUuid>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QHash>
#include <QMap>

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

#endif // STABLE_H




