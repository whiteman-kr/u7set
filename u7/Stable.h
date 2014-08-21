#pragma once

// C includes, must be before c++ includes
//
#include <assert.h>
#include <stdint.h>


// C++ includes
//
#include <memory>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <type_traits>


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
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>


// Qt Widgets
//
#include <QMainWindow>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
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

// Disable some warnings
//
#ifdef Q_OS_WIN
	#pragma warning(disable : 4482)		// nonstandard extension used: enum 'enum' used in qualified name
#endif


