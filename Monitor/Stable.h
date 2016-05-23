#ifndef MONITOR_STABLE_H
#define MONITOR_STABLE_H

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
#include <QApplication>
#include <QGuiApplication>
#include <QSharedMemory>
#include <QThread>
#include <QDebug>
#include <QtNetwork/QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QPoint>
#include <QUuid>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QJSEngine>
#include <QQmlEngine>
#include <QHash>
#include <QMap>


// Qt Widgets
//
#include <QMainWindow>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QScrollArea>
#include <QScrollBar>

#include <QListWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QTreeWidget>

#include <QHeaderView>
#include <QListView>

#include <QSplitter>
#include <QTextEdit>

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QFileDialog>
#include <QErrorMessage>

#include <QAbstractItemModel>
#include <QAbstractTableModel>

#include <QCloseEvent>

#include <QPainter>

// Qt XML
//
#include <QDomDocument>

//// Qt Sql
////
//#include <QSql>
//#include <QSqlDatabase>
//#include <QSqlError>
//#include <QSqlQuery>
//#include <QSqlDriver>


// Other stable includes
//
#include "../VFrame30/VFrame30Lib_global.h"
//#include "../include/Factory.h"
//#include "../include/TypesAndEnums.h"
//#include "../include/CUtils.h"

#include "../include/Types.h"

// Disable some warnings
//
#ifdef Q_OS_WIN
#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name

	// Disable 4996 warning
#ifndef _SCL_SECURE_NO_WARNINGS
	#define _SCL_SECURE_NO_WARNINGS
#endif
#endif

// For detecting memory leaks
//
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
   #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
   #define new DBG_NEW
#endif
#endif


#include "../include/AppSignalManager.h"

extern AppSignalManager theSignals;


#endif // MONITOR_STABLE_H

