#include "Settings.h"

QColor redColor = QColor(192, 0, 0);

//
// TuningClientAppSettings
//

TuningClientAppSettings::TuningClientAppSettings()
{
	// Determine the Local settings folder
	//
	m_localAppDataPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	QDir dir(m_localAppDataPath);
	if (dir.exists() == false)
	{
		dir.mkpath(m_localAppDataPath);
	}
}

TuningClientAppSettings& TuningClientAppSettings::instance()
{
	static TuningClientAppSettings theSettings;
	return theSettings;
}

void TuningClientAppSettings::save() const
{
	QSettings s{qApp->organizationName(), qApp->applicationName()};	// Explicitly point app name, as it can be changed via settings.
	saveSystem(s);
	return;
}

void TuningClientAppSettings::load()
{
	QSettings s{qApp->organizationName(), qApp->applicationName()};	// Explicitly point app name, as it can be changed via settings.
	loadSystem(s);
	m_wasLoadedFromFile = false;
	return;
}

bool TuningClientAppSettings::saveToFile(QString fileName) const
{
	QSettings s{fileName, QSettings::IniFormat};
	saveSystem(s);
	s.sync();
	return s.status() == QSettings::Status::NoError;
}

bool TuningClientAppSettings::loadFromFile(QString fileName)
{
	QSettings s{fileName, QSettings::IniFormat};
	loadSystem(s);
	m_wasLoadedFromFile = true;
	return s.status() == QSettings::Status::NoError;
}

bool TuningClientAppSettings::wasLoadedFromFile() const
{
	return m_wasLoadedFromFile;
}

void TuningClientAppSettings::saveSystem(QSettings& s) const
{
	s.setValue("TuningClientAppSettings/m_instanceStrId", m_systemData.m_instanceStrId);

	s.setValue("TuningClientAppSettings/m_configuratorIpAddress1", m_systemData.m_configuratorIpAddress1);
	s.setValue("TuningClientAppSettings/m_configuratorPort1", m_systemData.m_configuratorPort1);

	s.setValue("TuningClientAppSettings/m_configuratorIpAddress2", m_systemData.m_configuratorIpAddress2);
	s.setValue("TuningClientAppSettings/m_configuratorPort2", m_systemData.m_configuratorPort2);

	s.setValue("TuningClientAppSettings/m_filtersCustomFile", m_systemData.m_filtersCustomFile);
	s.setValue("TuningClientAppSettings/m_useFiltersCustomFile", m_systemData.m_useFiltersCustomFile);
	
	s.setValue("TuningClientAppSettings/m_language", m_systemData.m_language);
}

void TuningClientAppSettings::loadSystem(const QSettings& s)
{
	// Read system settings
	//
	m_systemData.m_instanceStrId = s.value("TuningClientAppSettings/m_instanceStrId", "SYSTEM_RACKID_WS00_TUN").toString();

	m_systemData.m_configuratorIpAddress1 = s.value("TuningClientAppSettings/m_configuratorIpAddress1", "127.0.0.1").toString();
	m_systemData.m_configuratorPort1 = s.value("TuningClientAppSettings/m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_systemData.m_configuratorIpAddress2 = s.value("TuningClientAppSettings/m_configuratorIpAddress2", "127.0.0.1").toString();
	m_systemData.m_configuratorPort2 = s.value("TuningClientAppSettings/m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

    m_systemData.m_filtersDefaultFile = QDir::toNativeSeparators(QObject::tr("%1/%2/UserFilters.xml")
                                                        .arg(localAppDataPath())
                                                        .arg(m_systemData.m_instanceStrId));

	m_systemData.m_filtersCustomFile = s.value("TuningClientAppSettings/m_filtersCustomFile", m_systemData.m_filtersDefaultFile).toString();
	m_systemData.m_useFiltersCustomFile = s.value("TuningClientAppSettings/m_useFiltersCustomFile", m_systemData.m_useFiltersCustomFile).toBool();

	m_systemData.m_language = s.value("TuningClientAppSettings/m_language", m_systemData.m_language).toString();
}


void TuningClientAppSettings::saveUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	s.setValue("MainWindow/pos", m_userData.m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_userData.m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_userData.m_mainWindowState);

	s.setValue("TuningWorkspace/Splitter/state", m_userData.m_tuningWorkspaceSplitterState);
	s.setValue("SchemasWorkspace/Splitter/state", m_userData.m_schemasWorkspaceSplitterState);

	s.setValue("PresetProperties/splitterState", m_userData.m_presetPropertiesSplitterState);
	s.setValue("PresetProperties/pos", m_userData.m_presetPropertiesWindowPos);
	s.setValue("PresetProperties/geometry", m_userData.m_presetPropertiesWindowGeometry);


	// Preset editor

	s.setValue("DialogFiltersEditor/pos", m_userData.m_dialogFiltersEditorPos);
	s.setValue("DialogFiltersEditor/geometry", m_userData.m_dialogFiltersEditorGeometry);

	s.setValue("DialogFiltersEditor/splitterPosition", m_userData.m_dialogFiltersEditorSplitterPosition);
	s.setValue("DialogFiltersEditor/propertyEditorSplitterPosition", m_userData.m_dialogFiltersEditorPropertyEditorSplitterPosition);

	//	SwitchPresetsPage options

	s.setValue("SwitchPresetsPage/ColCount", m_userData.m_switchPresetsPageColCount);
	s.setValue("SwitchPresetsPage/RowCount", m_userData.m_switchPresetsPageRowCount);
	s.setValue("SwitchPresetsPage/ButtonsWidth", m_userData.m_switchPresetsPageButtonsWidth);
	s.setValue("SwitchPresetsPage/ButtonsHeight", m_userData.m_switchPresetsPageButtonsHeight);

	s.setValue("SwitchPresetsPage/MainSplitterPosition", m_userData.m_switchPresetsPageSplitterPosition);

	//
	s.setValue("TuningWorkspace/Masks", m_userData.m_tuningWorkspaceMasks);

}

void TuningClientAppSettings::loadUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	m_userData.m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_userData.m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_userData.m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_userData.m_tuningWorkspaceSplitterState = s.value("TuningWorkspace/Splitter/state").toByteArray();
	m_userData.m_schemasWorkspaceSplitterState = s.value("SchemasWorkspace/Splitter/state").toByteArray();

	m_userData.m_presetPropertiesSplitterState = s.value("PresetProperties/splitterState").toInt();
	if (m_userData.m_presetPropertiesSplitterState < 100)
		m_userData.m_presetPropertiesSplitterState = 100;
	m_userData.m_presetPropertiesWindowPos = s.value("PresetProperties/pos", QPoint(-1, -1)).toPoint();
	m_userData.m_presetPropertiesWindowGeometry = s.value("PresetProperties/geometry").toByteArray();

	// Preset Editor

	m_userData.m_dialogFiltersEditorPos = s.value("DialogFiltersEditor/pos", QPoint(-1, -1)).toPoint();
	m_userData.m_dialogFiltersEditorGeometry = s.value("DialogFiltersEditor/geometry").toByteArray();

	m_userData.m_dialogFiltersEditorSplitterPosition = s.value("DialogFiltersEditor/splitterPosition").toByteArray();
	m_userData.m_dialogFiltersEditorPropertyEditorSplitterPosition = s.value("DialogFiltersEditor/propertyEditorSplitterPosition").toInt();

	//	SwitchPresetsPage options

	m_userData.m_switchPresetsPageColCount = s.value("SwitchPresetsPage/ColCount", m_userData.m_switchPresetsPageColCount).toInt();
	if (m_userData.m_switchPresetsPageColCount < 1) m_userData.m_switchPresetsPageColCount = 1;
	if (m_userData.m_switchPresetsPageColCount > 25) m_userData.m_switchPresetsPageColCount = 25;

	m_userData.m_switchPresetsPageRowCount = s.value("SwitchPresetsPage/RowCount", m_userData.m_switchPresetsPageRowCount).toInt();
	if (m_userData.m_switchPresetsPageRowCount < 1) m_userData.m_switchPresetsPageRowCount = 1;
	if (m_userData.m_switchPresetsPageRowCount > 25) m_userData.m_switchPresetsPageRowCount = 25;

	m_userData.m_switchPresetsPageButtonsWidth = s.value("SwitchPresetsPage/ButtonsWidth", m_userData.m_switchPresetsPageButtonsWidth).toInt();
	if (m_userData.m_switchPresetsPageButtonsWidth < 25) m_userData.m_switchPresetsPageButtonsWidth = 25;
	if (m_userData.m_switchPresetsPageButtonsWidth > 500) m_userData.m_switchPresetsPageButtonsWidth = 500;

	m_userData.m_switchPresetsPageButtonsHeight = s.value("SwitchPresetsPage/ButtonsHeight", m_userData.m_switchPresetsPageButtonsHeight).toInt();
	if (m_userData.m_switchPresetsPageButtonsHeight < 25) m_userData.m_switchPresetsPageButtonsHeight = 25;
	if (m_userData.m_switchPresetsPageButtonsHeight > 500) m_userData.m_switchPresetsPageButtonsHeight = 500;

	m_userData.m_switchPresetsPageSplitterPosition = s.value("SwitchPresetsPage/MainSplitterPosition").toByteArray();

	//

	m_userData.m_tuningWorkspaceMasks = s.value("TuningWorkspace/Masks").toStringList();
}

QString TuningClientAppSettings::localAppDataPath()
{
	return m_localAppDataPath;
}

const TuningClientAppSettings::SystemData& TuningClientAppSettings::system() const
{
	return m_systemData;
}

TuningClientAppSettings::SystemData& TuningClientAppSettings::system()
{
	return m_systemData;
}

void TuningClientAppSettings::setSystem(const TuningClientAppSettings::SystemData& src)
{
	m_systemData = src;
}
	
const TuningClientAppSettings::UserData& TuningClientAppSettings::user() const
{
	return m_userData;
}

TuningClientAppSettings::UserData& TuningClientAppSettings::user()
{
	return m_userData;
}

void TuningClientAppSettings::setUser(const TuningClientAppSettings::UserData& src)
{
	m_userData = src;
}

QString TuningClientAppSettings::instanceStrId()
{
	return m_systemData.m_instanceStrId;
}

HostAddressPort TuningClientAppSettings::configuratorAddress1()
{
	return HostAddressPort(m_systemData.m_configuratorIpAddress1, m_systemData.m_configuratorPort1);
}

HostAddressPort TuningClientAppSettings::configuratorAddress2()
{
	return HostAddressPort(m_systemData.m_configuratorIpAddress2, m_systemData.m_configuratorPort2);
}


QString TuningClientAppSettings::language() const
{
	return m_systemData.m_language;
}

// Returns default of custom file depending on useCustom flag
//
QString TuningClientAppSettings::userFiltersFile()
{
	if (useCustomFiltersFile() == true)
	{
		return customFiltersFile();
	}
	return m_systemData.m_filtersDefaultFile;
}

bool TuningClientAppSettings::useCustomFiltersFile()
{
	return m_systemData.m_useFiltersCustomFile;
}

QString TuningClientAppSettings::customFiltersFile()
{
	return m_systemData.m_filtersCustomFile;
}
