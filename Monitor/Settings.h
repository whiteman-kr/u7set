#pragma once

#include <../lib/SocketIO.h>
#include "MonitorSignalSnapshot.h"
#include "../lib/Ui/DialogSignalSearch.h"

class Settings
{
public:
	Settings();
	virtual ~Settings();

	Settings& operator = (const Settings& src);

	// Public methods
	//
public:
	void write() const;
	void load();

	void writeUserScope() const;
	void loadUserScope();

	void writeSystemScope() const;
	void loadSystemScope();

	// Properties
	//
public:
	QString instanceStrId() const;
	void setInstanceStrId(QString value);

	HostAddressPort configuratorAddress1() const;
	HostAddressPort configuratorAddress2() const;

	QString configuratorIpAddress1() const;
	void setConfiguratorIpAddress1(QString configuratorIpAddress);

	int configuratorPort1() const;
	void setConfiguratorPort1(int configuratorPort);

	QString configuratorIpAddress2() const;
	void setConfiguratorIpAddress2(QString configuratorIpAddress);

	int configuratorPort2() const;
	void setConfiguratorPort2(int configuratorPort);

	int requestTimeInterval() const;
	void setRequestTimeInterval(int value);

	bool showLogo() const;
	void setShowLogo(bool value);

	bool showItemsLabels() const;
	void setShowItemsLabels(bool value);

	bool singleInstance() const;
	void setSingleInstance(bool value);

	// Data	-- DO NOT FORGET TO ADD NEW MEMBERS TO ASSIGN OPERATOR
	//
public:

	// MainWindow settings -- user scope
	//
	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	// TrendSignals
	//
	QStringList m_trendSignalsDialogFilterCompleter;

	// Archive window
	//
	QPoint m_archiveWindowPos;
	QByteArray m_archiveWindowGeometry;
	QByteArray m_archiveWindowState;
	QByteArray m_archiveHorzHeader;
	int m_archiveHorzHeaderCount = 0;	// Stores ArchiveColumns::ColumnCount constant to restore default settings if columns set changes
	int m_archiveTimeType = static_cast<int>(E::TimeType::Local);
	QStringList m_archiveSignalsDialogFilterCompleter;

private:
	mutable QMutex m_mutex;

	QString m_instanceStrId;

	// --
	//
	QString m_configuratorIpAddress1;
	int m_configuratorPort1;

	QString m_configuratorIpAddress2;
	int m_configuratorPort2;

	int m_requestTimeInterval = 20;	// 20 ms

	bool m_showLogo = true;
	bool m_showItemsLabels = false;
	bool m_singleInstance = false;
};

extern Settings theSettings;


