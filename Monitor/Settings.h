#pragma once

#include <QMutex>
#include <../lib/SocketIO.h>

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

	// Data	-- DO NOT FORGET TO ADD NEW MEMBERS TO ASSIGN OPERATOR
	//
public:

	// MainWindow settings -- user scope
	//
	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

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
};

extern Settings theSettings;


