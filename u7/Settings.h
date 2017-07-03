#pragma once

#include <QString>
#include <QStringList>
#include <QPoint>

struct DatabaseConnectionParam
{
	QChar m_address[256];
	qint32 m_port;
	QChar m_login[256];
	QChar m_password[256];

	QString address() const;
	void setAddress(QString str);

	int port() const;
	void setPort(int port);

	QString login() const;
	void setLogin(QString str);

	QString password() const;
	void setPassword(QString str);
};


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
	QString serverIpAddress() const;
	void setServerIpAddress(const QString& value);

	int serverPort() const;
	void setServerPort(int value);

	QString serverUsername() const;
	void setServerUsername(const QString& value);

	QString serverPassword() const;
	void setServerPassword(const QString& value);

	const QString& buildOutputPath() const;
	void setBuildOutputPath(const QString& value);

	const QStringList& loginCompleter() const;
	QStringList& loginCompleter();

	bool freezeBuildPath() const;

	void setDebugMode(bool value);
	bool debugMode() const;
	bool isDebugMode() const;

	bool isExpertMode() const;
	void setExpertMode(bool value);

	bool isInfoMode() const;
	bool infoMode() const;
	void setInfoMode(bool value);


	int buildWarningLevel() const;
	void setBuildWarningLevel(int value);

	const QStringList& buildSearchCompleter() const;
	QStringList& buildSearchCompleter();

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

    int m_equipmentTabPagePropertiesSplitterState = 0;

	// Signals Tab Page
	//
	//QByteArray m_equipmentTabPageSplitterState;

	// Build Tab Page
	//
	QByteArray m_buildTabPageSplitterState;

    // Text Editor options
    //
    QPoint m_DialogTextEditorWindowPos;
    QByteArray m_DialogTextEditorWindowGeometry;

	// Property Editor Options
	//
	QPoint m_multiLinePropertyEditorWindowPos;
	QByteArray m_multiLinePropertyEditorGeometry;

	QPoint m_scriptHelpWindowPos;
	QByteArray m_scriptHelpWindowGeometry;

	// Connection editor
	//
	QPoint m_connectionEditorWindowPos;
	QByteArray m_connectionEditorWindowGeometry;
    QByteArray m_connectionEditorSplitterState;
    int m_connectionEditorPeSplitterPosition = 0;
	int m_connectionEditorSortColumn;
	Qt::SortOrder m_connectionEditorSortOrder;
	QStringList m_connectionEditorMasks;

	// Bus Editor
	QPoint m_busEditorWindowPos;
	QByteArray m_busEditorWindowGeometry;
	QByteArray m_busEditorSplitterState;
	QPoint m_busEditorPeWindowPos;
	QByteArray m_busEditorPeWindowGeometry;
	int m_busEditorPeSplitterPosition = 0;
	int m_busEditorSortColumn;
	Qt::SortOrder m_busEditorSortOrder;


	// CreateSchema dialog
	//
	QString m_lastSelectedLmDescriptionFile;

	// SchemaItem properties
    //
    QPoint m_schemaItemPropertiesWindowPos;
    QByteArray m_schemaItemPropertiesWindowGeometry;
    int m_schemaItemSplitterState = 0;

	// Configurator properties
	//
	QString m_configuratorSerialPort;
	bool m_configuratorShowDebugInfo = false;
	bool m_configuratorVerify = true;
	QByteArray m_UploadTabPageSplitterState;

private:
	DatabaseConnectionParam m_databaseConnection;
	QString m_buildOutputPath;
	bool m_expertMode = false;

	QStringList m_loginCompleter;

	bool m_debugMode = false;
	bool m_infoMode = false;
	int m_buildWarningLevel = 0;		// 0 is Show All Warnings
	QStringList m_buildSerachCompleter;
};

extern Settings theSettings;


