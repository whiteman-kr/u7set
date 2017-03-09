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
#include <iterator>


// Qt includes
//
#include <QApplication>
#include <QGuiApplication>
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
#include <QStringList>


// Qt Widgets
//
#include <QMainWindow>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QScrollArea>
#include <QScrollBar>

#include <QListWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QTreeWidget>

#include <QHeaderView>
#include <QListView>

#include <QSplitter>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QFileDialog>
#include <QErrorMessage>
#include <QDialogButtonBox>

#include <QAbstractItemModel>
#include <QAbstractTableModel>

#include <QCloseEvent>

#include <QPainter>

#include <QClipboard>


// Qt Sql
//
#include <QSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>


// Other stable includes
//
#include "../VFrame30/VFrame30Lib_global.h"
#include "../lib/Factory.h"
#include "../lib/TypesAndEnums.h"
#include "../lib/CUtils.h"

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
#include <crtdbg.h>
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif

#endif // STABLE_H




