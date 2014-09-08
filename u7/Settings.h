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
	const QString& serverIpAddress() const;
	void setServerIpAddress(const QString& value);

	int serverPort() const;
	void setServerPort(int value);

	const QString& serverUsername() const;
	void setServerUsername(const QString& value);

	const QString& serverPassword() const;
	void setServerPassword(const QString& value);

	// Data
	//
public:

	// MainWindow settings -- user scope
	//
	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	// LoginDialog settings -- user scope
	//
	QString loginDialog_defaultUsername;

	// Configurations Tab Page
	//
	QByteArray m_configurationTabPageSplitterState;

	// Equipment Tab Page
	//
	QByteArray m_equipmentTabPageSplitterState;

	// Signals Tab Page
	//
	//QByteArray m_equipmentTabPageSplitterState;

private:
	// --
	//
	QString m_serverIpAddress;
	int m_serverPort;
	QString m_serverUsername;
	QString m_serverPassword;
};

extern Settings theSettings;


