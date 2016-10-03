#pragma once

#include <QString>
#include <QStringList>
#include <QPoint>

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

	const QString& buildOutputPath() const;
	void setBuildOutputPath(const QString& value);

	const QStringList& loginCompleter() const;
	QStringList& loginCompleter();

	bool freezeBuildPath() const;
	bool useConnections() const;

	void setDebugMode(bool value);
	bool debugMode() const;
	bool isDebugMode() const;

	bool isExpertMode() const;
	void setExpertMode(bool value);

	bool isInfoMode() const;
	bool infoMode() const;
	void setInfoMode(bool value);

	bool hideWarnings() const;
	void setHideWarnings(bool value);

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

    // AFBL Editor options
    //
    QPoint m_abflEditorWindowPos;
    QByteArray m_abflEditorWindowGeometry;

    // AFBL Editor options
    //
    QPoint m_abflPropertiesWindowPos;
    QByteArray m_abflPropertiesWindowGeometry;

    // Text Editor options
    //
    QPoint m_DialogTextEditorWindowPos;
    QByteArray m_DialogTextEditorWindowGeometry;

	// Property Editor Options
	//
	QPoint m_multiLinePropertyEditorWindowPos;
	QByteArray m_multiLinePropertyEditorGeometry;

	//Connection editor
	//
	QPoint m_connectionEditorWindowPos;
	QByteArray m_connectionEditorWindowGeometry;
	int m_connectionEditorSortColumn;
	Qt::SortOrder m_connectionEditorSortOrder;
	int m_connectionEditorMaskType;
	QStringList m_connectionEditorMasks;

	//Connection properties
    //
    QPoint m_connectionPropertiesWindowPos;
    QByteArray m_connectionPropertiesWindowGeometry;
    int m_connectionSplitterState = 0;

	//SchemaItem properties
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
	// --
	//
	QString m_serverIpAddress;
	int m_serverPort = 0;
	QString m_serverUsername;
	QString m_serverPassword;
	QString m_buildOutputPath;
	bool m_expertMode = false;

	QStringList m_loginCompleter;

	bool m_useConnections = true;

	bool m_debugMode = false;
	bool m_infoMode = false;
	bool m_hideWarnings = false;
	QStringList m_buildSerachCompleter;

};

extern Settings theSettings;


