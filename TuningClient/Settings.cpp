#include "Settings.h"
#include "../lib/SocketIO.h"


//
// ConfigConnection
//


ConfigConnection::ConfigConnection(QString EquipmentId, QString ipAddress, int port) :
	m_equipmentId(EquipmentId),
	m_ip(ipAddress),
	m_port(port)
{
}

QString ConfigConnection::equipmentId() const
{
	return m_equipmentId;
}

QString ConfigConnection::ip() const
{
	return m_ip;
}

int ConfigConnection::port() const
{
	return m_port;
}

HostAddressPort ConfigConnection::address() const
{
	HostAddressPort h(m_ip, m_port);
	return h;
}

//
// Settings
//

Settings::Settings():
	m_instanceStrId("SYSTEMID_WS00_TUN"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_REQUEST),
	m_language("en")
{
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
}

void Settings::StoreSystem()
{
	if (admin() == false)
	{
		return;
	}

	QSettings s(QSettings::SystemScope, qApp->organizationName(), qApp->applicationName());

	s.setValue("m_instanceStrId", m_instanceStrId);

	s.setValue("m_configuratorIpAddress1", m_configuratorIpAddress1);
	s.setValue("m_configuratorPort1", m_configuratorPort1);

	s.setValue("m_configuratorIpAddress2", m_configuratorIpAddress2);
	s.setValue("m_configuratorPort2", m_configuratorPort2);
}

void Settings::RestoreSystem()
{
	// determine if is running as administrator
	//
	QSettings adminSettings(QSettings::SystemScope, qApp->organizationName(), qApp->applicationName());
	adminSettings.setValue("ApplicationName", qApp->applicationName());

	adminSettings.sync();

	if (adminSettings.status() == QSettings::AccessError)
	{
		m_admin = false;
	}
	else
	{
		m_admin = true;
	}

	// read system settings
	//
	QSettings s(QSettings::SystemScope, qApp->organizationName(), qApp->applicationName());

	m_instanceStrId = s.value("m_instanceStrId", "SYSTEM_RACKID_WS00_TUN").toString();

	m_configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
	m_configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_REQUEST).toInt();

	m_configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
	m_configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_REQUEST).toInt();

	// Determine the Global settings folder

	QSettings qs(QSettings::IniFormat, QSettings::SystemScope, qApp->organizationName(), qApp->applicationName());

	QString m_globalAppDataPath = QDir::toNativeSeparators(qs.fileName());

	int ptPos = m_globalAppDataPath.indexOf('.');
	if (ptPos != -1)
	{
		m_globalAppDataPath = m_globalAppDataPath.left(ptPos);
	}

	qDebug() << m_globalAppDataPath;

	if (m_admin == true)
	{
		QDir dir(m_globalAppDataPath);

		if (dir.exists() == false)
		{
			dir.mkpath(m_globalAppDataPath);
		}
	}

	// Determine the Local settings folder

	m_localAppDataPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	qDebug() << m_localAppDataPath;

	QDir dir(m_localAppDataPath);

	if (dir.exists() == false)
	{
		dir.mkpath(m_localAppDataPath);
	}

	m_userFiltersFile = QDir::toNativeSeparators(m_localAppDataPath + "/UserFilters.xml");

}


void Settings::StoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("TuningWorkspace/Splitter/state", m_tuningWorkspaceSplitterState);
	s.setValue("SchemasWorkspace/Splitter/state", m_schemasWorkspaceSplitterState);

	s.setValue("TuningPage/Count", static_cast<uint>(m_tuningPageSettings.size()));
	for (int i = 0; i < m_tuningPageSettings.size(); i++)
	{
		s.setValue(QString("TuningPage/Settings%1/columnCount").arg(i), m_tuningPageSettings[i].m_columnCount);
		for (int c = 0; c < m_tuningPageSettings[i].m_columnCount; c++)
		{
			s.setValue(QString("TuningPage/Settings%1/columnWidth/%2").arg(i).arg(c), m_tuningPageSettings[i].m_columnsWidth[c]);
			s.setValue(QString("TuningPage/Settings%1/columnIndex/%2").arg(i).arg(c), m_tuningPageSettings[i].m_columnsIndexes[c]);
		}
	}

	s.setValue("PropertyEditor/multiLinePos", m_multiLinePropertyEditorWindowPos);
	s.setValue("PropertyEditor/multiLineGeometry", m_multiLinePropertyEditorGeometry);

	s.setValue("PresetProperties/splitterState", m_presetPropertiesSplitterState);
	s.setValue("PresetProperties/pos", m_presetPropertiesWindowPos);
	s.setValue("PresetProperties/geometry", m_presetPropertiesWindowGeometry);

	// Preset editor

    s.setValue("TuningFiltersEditor/pos", m_presetEditorPos);
    s.setValue("TuningFiltersEditor/geometry", m_presetEditorGeometry);

    s.setValue("TuningFiltersEditor/SignalsTableColumnWidth", QVariant::fromValue(m_presetEditorSignalsTableColumnWidth));
    s.setValue("TuningFiltersEditor/PresetsTreeColumnWidth", QVariant::fromValue(m_presetEditorPresetsTreeColumnWidth));
    s.setValue("TuningFiltersEditor/PropertyEditorSplitterPos", m_presetEditorPropertyEditorSplitterPos);
    s.setValue("TuningFiltersEditor/PropertyEditorGeometry", m_presetEditorPropertyEditorGeometry);

	//

	s.setValue("MainWindow/language", m_language);

}

void Settings::RestoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_tuningWorkspaceSplitterState = s.value("TuningWorkspace/Splitter/state").toByteArray();
	m_schemasWorkspaceSplitterState = s.value("SchemasWorkspace/Splitter/state").toByteArray();

	int tuningPageSettingsCount = s.value("TuningPage/Count", 0).toInt();
	m_tuningPageSettings.resize(tuningPageSettingsCount);

	for (int i = 0; i < tuningPageSettingsCount; i++)
	{
		m_tuningPageSettings[i].m_columnCount = s.value(QString("TuningPage/Settings%1/columnCount").arg(i), 0).toInt();
		m_tuningPageSettings[i].m_columnsWidth.resize(m_tuningPageSettings[i].m_columnCount);
		m_tuningPageSettings[i].m_columnsIndexes.resize(m_tuningPageSettings[i].m_columnCount);
		for (int c = 0; c < m_tuningPageSettings[i].m_columnCount; c++)
		{
			m_tuningPageSettings[i].m_columnsWidth[c] = s.value(QString("TuningPage/Settings%1/columnWidth/%2").arg(i).arg(c), 100).toInt();
			m_tuningPageSettings[i].m_columnsIndexes[c] = s.value(QString("TuningPage/Settings%1/columnIndex/%2").arg(i).arg(c), 0).toInt();
		}
	}

	m_multiLinePropertyEditorWindowPos = s.value("PropertyEditor/multiLinePos", QPoint(-1, -1)).toPoint();
	m_multiLinePropertyEditorGeometry = s.value("PropertyEditor/multiLineGeometry").toByteArray();

	m_presetPropertiesSplitterState = s.value("PresetProperties/splitterState").toInt();
	if (m_presetPropertiesSplitterState < 100)
		m_presetPropertiesSplitterState = 100;
	m_presetPropertiesWindowPos = s.value("PresetProperties/pos", QPoint(-1, -1)).toPoint();
	m_presetPropertiesWindowGeometry = s.value("PresetProperties/geometry").toByteArray();

	// Preset Editor

    m_presetEditorPos = s.value("TuningFiltersEditor/pos", QPoint(-1, -1)).toPoint();
    m_presetEditorGeometry = s.value("TuningFiltersEditor/geometry").toByteArray();

    m_presetEditorSignalsTableColumnWidth = s.value("TuningFiltersEditor/SignalsTableColumnWidth").value<QList<int>>();
    m_presetEditorPresetsTreeColumnWidth = s.value("TuningFiltersEditor/PresetsTreeColumnWidth").value<QList<int> >();
    m_presetEditorPropertyEditorSplitterPos = s.value("TuningFiltersEditor/PropertyEditorSplitterPos").toInt();
    m_presetEditorPropertyEditorGeometry = s.value("TuningFiltersEditor/PropertyEditorGeometry").toByteArray();

	//

	m_language = s.value("MainWindow/language", m_language).toString();
}

QString Settings::instanceStrId()
{
	QMutexLocker l(&m);
	return m_instanceStrId;
}

void Settings::setInstanceId(const QString& value)
{
	QMutexLocker l(&m);
	m_instanceStrId = value;
}

HostAddressPort Settings::configuratorAddress1()
{
	QMutexLocker l(&m);
	return HostAddressPort(m_configuratorIpAddress1, m_configuratorPort1);
}

void Settings::setConfiguratorAddress1(const QString& address, int port)
{
	m_configuratorIpAddress1 = address;
	m_configuratorPort1 = port;
}

HostAddressPort Settings::configuratorAddress2()
{
	QMutexLocker l(&m);
	return HostAddressPort(m_configuratorIpAddress2, m_configuratorPort2);
}

void Settings::setConfiguratorAddress2(const QString& address, int port)
{
	m_configuratorIpAddress2 = address;
	m_configuratorPort2 = port;
}


QString Settings::language() const
{
	return m_language;
}

void Settings::setLanguage(const QString& value)
{
	m_language = value;
}

bool Settings::admin() const
{
	return m_admin;
}

TuningPageSettings* Settings::tuningPageSettings(int index)
{
	// Reserve place for tuning page settings and copy existing
	//
	if (index >= m_tuningPageSettings.size())
	{
		std::vector<TuningPageSettings> m_tuningPageSettings2 = m_tuningPageSettings;

		m_tuningPageSettings.resize(index + 1);
		for (int i = 0; i < m_tuningPageSettings2.size(); i++)
		{
			m_tuningPageSettings[i] = m_tuningPageSettings2[i];
		}
	}

	if (index >= m_tuningPageSettings.size())
	{
		assert(false);
		return nullptr;
	}

	return& m_tuningPageSettings[index];
}

QString Settings::globalAppDataPath()
{
	return m_globalAppDataPath;
}

QString Settings::localAppDataPath()
{
	return m_localAppDataPath;
}

QString Settings::userFiltersFile()
{
	return m_userFiltersFile;
}

Settings theSettings;

ConfigSettings theConfigSettings;
