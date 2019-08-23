#pragma once

#include <memory>
#include <algorithm>
#include <set>
#include <assert.h>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 6326)		// MSVC warning C6326: potential comparison of a constant with another constant
#endif

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QSplitter>
#include <QTreeWidget>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QListView>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>
#include <QLabel>
#include <QStatusBar>
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QDomDocument>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QButtonGroup>
#include <QUuid>
#include <QStandardPaths>
#include <QDir>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QClipboard>
#include <QSharedMemory>
#include <QCryptographicHash>
#include <QPainter>
#include <QScrollArea>
#include <QMouseEvent>
#include <QtQml>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#include "../VFrame30/VFrame30Lib_global.h"
#include "../lib/TypesAndEnums.h"


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

