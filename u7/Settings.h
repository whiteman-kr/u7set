#pragma once

using maptype = QMap<QString, int>;
Q_DECLARE_METATYPE(maptype)


class Settings
{
public:
	Settings();
	virtual ~Settings();

	// Public methods
	//
public:
	void load();

	void writeUserScope() const;
	void loadUserScope();

	// Properties
	//
public:
	const QStringList& loginCompleter() const;
	QStringList& loginCompleter();

	bool freezeBuildPath() const;

	void setDebugMode(bool value);
	bool debugMode() const;
	bool isDebugMode() const;

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
	QByteArray m_mainWindowState; // Toolbars/dock's

	// LoginDialog settings -- user scope
	//
	QString loginDialog_defaultUsername;

	// Projects Tab Page
	//
	int m_projectsSortColumn = 0;
	Qt::SortOrder m_projectsSortOrder = Qt::AscendingOrder;

	// Configurations Tab Page
	//
	QByteArray m_configurationTabPageSplitterState;

	// Build Tab Page
	//
	QByteArray m_buildTabPageSplitterState;

	// Text Editor options
	//
	QPoint m_DialogTextEditorWindowPos;
	QByteArray m_DialogTextEditorWindowGeometry;

	// Connection editor
	//
	QPoint m_connectionEditorWindowPos;
	QByteArray m_connectionEditorWindowGeometry;
	QByteArray m_connectionEditorSplitterState;
	int m_connectionEditorPeSplitterPosition = 0;
	int m_connectionEditorSortColumn = 0;
	Qt::SortOrder m_connectionEditorSortOrder = Qt::AscendingOrder;
	QStringList m_connectionEditorMasks;

	// Bus Editor
	//
	QPoint m_busEditorWindowPos;
	QByteArray m_busEditorWindowGeometry;
	QByteArray m_busEditorMainSplitterState;
	QByteArray m_busEditorRightSplitterState;
	int m_busEditorPropertySplitterPosition = 100;
	QSize m_busEditorPeWindowSize;
	int m_busEditorPeSplitterPosition = 100;
	int m_busEditorSortColumn = 0;
	Qt::SortOrder m_busEditorSortOrder = Qt::AscendingOrder;

	// Behavior Editor
	//
	int m_behaviorEditorSortColumn = 0;
	Qt::SortOrder m_behaviorEditorSortOrder = Qt::AscendingOrder;
	QByteArray m_behaviorEditorHSplitterState;
	QByteArray m_behaviorEditorVSplitterState;

	QByteArray m_afbLibratyCheckSplitterState;

	QByteArray m_specificEditorSplitterState;

	QByteArray m_svgEditorSplitterState;
	bool m_svgEditorStretch = true;

	// CreateSchema dialog
	//
	QString m_lastSelectedLmDescriptionFile;

	// SchemaItemPropertiesDialog

	int m_schemaItemPropertiesSplitterPosition = 100;
	QString m_schemaItemPropertiesPropertyFilter;
	bool m_schemaItemPropertiesExpandValuesToAllRows = true;
	QMap<QString, int> m_schemaItemPropertiesColumnsWidth;
	bool m_schemaItemPropertiesGroupByCategory = false;
	QByteArray m_schemaItemPropertiesGeometry;

	// Find SchemaItem
	//
	bool m_findSchemaItemCaseSensitive = false;

	// Find/Replace Dialog in IdeCodeEditor
	//
	QStringList m_findCompleter;
	QStringList m_replaceCompleter;

	// Configurator properties
	//
	QByteArray m_UploadTabPageLeftSplitterState;
	QByteArray m_UploadTabPageRightSplitterState;

	// DialogMetrologyConnection
	//
	QMap<QString, int> m_dialogMetrologyConnectionColumnsWidth;
	QByteArray m_dialogMetrologyConnectionGeometry;

private:
	QStringList m_loginCompleter;

	bool m_debugMode = false;
	bool m_infoMode = false;
	int m_buildWarningLevel = 0; // 0 is Show All Warnings
	QStringList m_buildSerachCompleter;
};

extern Settings theSettings;
