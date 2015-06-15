#pragma once

class Settings
{
public:
	Settings();
	virtual ~Settings();

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
	QString configuratorIpAddress() const;
	void setConfiguratorIpAddress(QString configuratorIpAddress);

	int configuratorPort() const;
	void setConfiguratorPort(int configuratorPort);

	// Data
	//
public:

	// MainWindow settings -- user scope
	//
	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's


private:
	// --
	//
	QString m_configuratorIpAddress;
	int m_configuratorPort;
};

extern Settings theSettings;


