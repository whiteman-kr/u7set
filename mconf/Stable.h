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


// Log
//
#include "../lib/OutputLog.h"

extern OutputLog theLog;
