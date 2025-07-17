#include "../u7/Settings.h"

Settings theSettings;

//
// QMap<QString, int> serialization stuff
//

#ifndef QT_NO_DATASTREAM
QDataStream& operator<<(QDataStream& stream, const QMap<QString, int>& map)
{
	QMap<QString, int>::const_iterator i = map.constBegin();
	while (i != map.constEnd())
	{
		if (i.key().isEmpty() == false)
		{
			stream << i.key() << i.value();
		}
		++i;
	}
	return stream;
}

QDataStream& operator>>(QDataStream& stream, QMap<QString, int>& map)
{
	QString key;
	int value = 0;

	map.clear();

	while (!stream.atEnd())
	{
		stream >> key;
		stream >> value;
		if (key.isEmpty() == false)
		{
			map[key] = value;
		}
	}
	return stream;
}

#endif

//
//	Settings
//
Settings::Settings() {}

Settings::~Settings() {}

void Settings::load()
{
	loadUserScope();
}

void Settings::writeUserScope() const
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("loginDialog_defaultUsername", loginDialog_defaultUsername);

	s.setValue("ProjectsTabPage/sortColumn", m_projectsSortColumn);
	s.setValue("ProjectsTabPage/sortOrder", static_cast<int>(m_projectsSortOrder));

	s.setValue("ConfigurationTabPage/Splitter/state", m_configurationTabPageSplitterState);

	s.setValue("BuildTabPage/Splitter/state", m_buildTabPageSplitterState);

	s.setValue("TextEditorProperties/pos", m_DialogTextEditorWindowPos);
	s.setValue("TextEditorProperties/geometry", m_DialogTextEditorWindowGeometry);

	s.setValue("LoginDialog/loginCompleter", m_loginCompleter);

	s.setValue("ConnectionEditor/pos", m_connectionEditorWindowPos);
	s.setValue("ConnectionEditor/geometry", m_connectionEditorWindowGeometry);
	s.setValue("ConnectionEditor/splitter", m_connectionEditorSplitterState);
	s.setValue("ConnectionEditor/peSplitterPos", m_connectionEditorPeSplitterPosition);
	s.setValue("ConnectionEditor/sortColumn", m_connectionEditorSortColumn);
	s.setValue("ConnectionEditor/sortOrder", static_cast<int>(m_connectionEditorSortOrder));
	s.setValue("ConnectionEditor/masks", m_connectionEditorMasks);

	s.setValue("BusEditor/pos", m_busEditorWindowPos);
	s.setValue("BusEditor/geometry", m_busEditorWindowGeometry);
	s.setValue("BusEditor/mainSplitter", m_busEditorMainSplitterState);
	s.setValue("BusEditor/rightSplitter", m_busEditorRightSplitterState);
	s.setValue("BusEditor/busPropertySplitter", m_busEditorPropertySplitterPosition);
	s.setValue("BusEditor/peSize", m_busEditorPeWindowSize);
	s.setValue("BusEditor/peSplitterPos", m_busEditorPeSplitterPosition);
	s.setValue("BusEditor/sortColumn", m_busEditorSortColumn);
	s.setValue("BusEditor/sortOrder", static_cast<int>(m_busEditorSortOrder));

	s.setValue("BehaviorEditor/sortColumn", m_behaviorEditorSortColumn);
	s.setValue("BehaviorEditor/sortOrder", static_cast<int>(m_behaviorEditorSortOrder));
	s.setValue("BehaviorEditor/hSplitterState", m_behaviorEditorHSplitterState);
	s.setValue("BehaviorEditor/vSplitterState", m_behaviorEditorVSplitterState);


	s.setValue("AfbLibraryCheck/mainSplitter", m_afbLibratyCheckSplitterState);

	s.setValue("SpecificEditor/mainSplitter", m_specificEditorSplitterState);
	s.setValue("SvgEditor/mainSplitter", m_svgEditorSplitterState);
	s.setValue("SvgEditor/stretch", m_svgEditorStretch);

	s.setValue("CreateSchema/lastSelectedLmDescriptionFile", m_lastSelectedLmDescriptionFile);

	s.setValue("SchemaItemPropertiesDialog/Splitter", m_schemaItemPropertiesSplitterPosition);
	s.setValue("SchemaItemPropertiesDialog/PropertiesMask", m_schemaItemPropertiesPropertyFilter);
	s.setValue("SchemaItemPropertiesDialog/ExpandValuesToAllRows", m_schemaItemPropertiesExpandValuesToAllRows);
	s.setValue("SchemaItemPropertiesDialog/ColumnsWidth", QVariant::fromValue(m_schemaItemPropertiesColumnsWidth));
	s.setValue("SchemaItemPropertiesDialog/GroupByCategory", m_schemaItemPropertiesGroupByCategory);
	s.setValue("SchemaItemPropertiesDialog/Geometry", m_schemaItemPropertiesGeometry);

	s.setValue("m_infoMode", m_infoMode);

	s.setValue("UploadTabPage/LeftSplitter/state", m_UploadTabPageLeftSplitterState);
	s.setValue("UploadTabPage/RightSplitter/state", m_UploadTabPageRightSplitterState);

	s.setValue("BuildTabPage/m_buildWarningLevel", m_buildWarningLevel);
	s.setValue("BuildTabPage/m_buildSerachCompleter", m_buildSerachCompleter);

	s.setValue("DialogMetrologyConnection/ColumnsWidth", QVariant::fromValue(m_dialogMetrologyConnectionColumnsWidth));
	s.setValue("DialogMetrologyConnection/Geometry", m_dialogMetrologyConnectionGeometry);

	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	loginDialog_defaultUsername = s.value("loginDialog_defaultUsername").toString();

	m_projectsSortColumn = s.value("ProjectsTabPage/sortColumn").toInt();
	m_projectsSortOrder = static_cast<Qt::SortOrder>(s.value("ProjectsTabPage/sortOrder").toInt());

	m_configurationTabPageSplitterState = s.value("ConfigurationTabPage/Splitter/state").toByteArray();

	m_buildTabPageSplitterState = s.value("BuildTabPage/Splitter/state").toByteArray();

	m_DialogTextEditorWindowPos = s.value("TextEditorProperties/pos", QPoint(-1, -1)).toPoint();
	m_DialogTextEditorWindowGeometry = s.value("TextEditorProperties/geometry").toByteArray();

	m_loginCompleter = s.value("LoginDialog/loginCompleter").toStringList();
	if (m_loginCompleter.isEmpty() == true)
	{
		m_loginCompleter << "Administrator";
	}

	//

	m_connectionEditorWindowPos = s.value("ConnectionEditor/pos", QPoint(-1, -1)).toPoint();
	m_connectionEditorWindowGeometry = s.value("ConnectionEditor/geometry").toByteArray();
	m_connectionEditorSplitterState = s.value("ConnectionEditor/splitter").toByteArray();
	m_connectionEditorPeSplitterPosition = s.value("ConnectionEditor/peSplitterPos").toInt();
	if (m_connectionEditorPeSplitterPosition < 150)
	{
		m_connectionEditorPeSplitterPosition = 150;
	}

	m_connectionEditorSortColumn = s.value("ConnectionEditor/sortColumn").toInt();
	m_connectionEditorSortOrder = static_cast<Qt::SortOrder>(s.value("ConnectionEditor/sortOrder").toInt());
	m_connectionEditorMasks = s.value("ConnectionEditor/masks").toStringList();

	//
	m_busEditorWindowPos = s.value("BusEditor/pos", QPoint(-1, -1)).toPoint();
	m_busEditorWindowGeometry = s.value("BusEditor/geometry").toByteArray();
	m_busEditorMainSplitterState = s.value("BusEditor/mainSplitter").toByteArray();
	m_busEditorRightSplitterState = s.value("BusEditor/rightSplitter").toByteArray();

	m_busEditorPropertySplitterPosition = s.value("BusEditor/busPropertySplitter").toInt();
	if (m_busEditorPropertySplitterPosition < 150)
	{
		m_busEditorPropertySplitterPosition = 150;
	}

	m_busEditorPeWindowSize = s.value("BusEditor/peSize").toSize();
	m_busEditorPeSplitterPosition = s.value("BusEditor/peSplitterPos").toInt();
	if (m_busEditorPeSplitterPosition < 150)
	{
		m_busEditorPeSplitterPosition = 150;
	}
	m_busEditorSortColumn = s.value("BusEditor/sortColumn").toInt();
	m_busEditorSortOrder = static_cast<Qt::SortOrder>(s.value("BusEditor/sortOrder").toInt());

	m_behaviorEditorSortColumn = s.value("BehaviorEditor/sortColumn").toInt();
	m_behaviorEditorSortOrder = static_cast<Qt::SortOrder>(s.value("BehaviorEditor/sortOrder").toInt());
	m_behaviorEditorHSplitterState = s.value("BehaviorEditor/hSplitterState").toByteArray();
	m_behaviorEditorVSplitterState = s.value("BehaviorEditor/vSplitterState").toByteArray();

	//

	m_afbLibratyCheckSplitterState = s.value("AfbLibraryCheck/mainSplitter").toByteArray();

	m_specificEditorSplitterState = s.value("SpecificEditor/mainSplitter").toByteArray();

	m_svgEditorSplitterState = s.value("SvgEditor/mainSplitter").toByteArray();
	m_svgEditorStretch = s.value("SvgEditor/stretch", m_svgEditorStretch).toBool();

	//

	m_lastSelectedLmDescriptionFile = s.value("CreateSchema/lastSelectedLmDescriptionFile", "").toString();

	//

	m_schemaItemPropertiesSplitterPosition = s.value("SchemaItemPropertiesDialog/Splitter").toInt();
	if (m_schemaItemPropertiesSplitterPosition < 100)
	{
		m_schemaItemPropertiesSplitterPosition = 100;
	}

	m_schemaItemPropertiesPropertyFilter = s.value("SchemaItemPropertiesDialog/PropertiesMask").toString();
	m_schemaItemPropertiesExpandValuesToAllRows =
		s.value("SchemaItemPropertiesDialog/ExpandValuesToAllRows", m_schemaItemPropertiesExpandValuesToAllRows).toBool();
	m_schemaItemPropertiesColumnsWidth = s.value("SchemaItemPropertiesDialog/ColumnsWidth").value<QMap<QString, int>>();
	m_schemaItemPropertiesGroupByCategory =
		s.value("SchemaItemPropertiesDialog/GroupByCategory", m_schemaItemPropertiesGroupByCategory).toBool();
	m_schemaItemPropertiesGeometry = s.value("SchemaItemPropertiesDialog/Geometry").toByteArray();

	m_infoMode = s.value("m_infoMode").toBool();

	m_UploadTabPageLeftSplitterState = s.value("UploadTabPage/LeftSplitter/state").toByteArray();
	m_UploadTabPageRightSplitterState = s.value("UploadTabPage/RightSplitter/state").toByteArray();

	m_buildWarningLevel = s.value("BuildTabPage/m_buildWarningLevel").toBool();
	m_buildSerachCompleter = s.value("BuildTabPage/m_buildSerachCompleter").toStringList();

	m_dialogMetrologyConnectionColumnsWidth = s.value("DialogMetrologyConnection/ColumnsWidth").value<QMap<QString, int>>();
	m_dialogMetrologyConnectionGeometry = s.value("DialogMetrologyConnection/Geometry").toByteArray();

	return;
}

const QStringList& Settings::loginCompleter() const
{
	return m_loginCompleter;
}

QStringList& Settings::loginCompleter()
{
	return m_loginCompleter;
}

void Settings::setDebugMode(bool value)
{
	m_debugMode = value;
}

bool Settings::debugMode() const
{
	return m_debugMode;
}

bool Settings::isDebugMode() const
{
	return debugMode();
}

bool Settings::isInfoMode() const
{
	return m_infoMode;
}

bool Settings::infoMode() const
{
	return m_infoMode;
}

void Settings::setInfoMode(bool value)
{
	m_infoMode = value;
}

int Settings::buildWarningLevel() const
{
	return m_buildWarningLevel;
}

void Settings::setBuildWarningLevel(int value)
{
	m_buildWarningLevel = value;
}

const QStringList& Settings::buildSearchCompleter() const
{
	return m_buildSerachCompleter;
}

QStringList& Settings::buildSearchCompleter()
{
	return m_buildSerachCompleter;
}
