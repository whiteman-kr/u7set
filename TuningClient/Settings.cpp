#include "Stable.h"
#include "Settings.h"
#include "../lib/SocketIO.h"

Settings::Settings():
	m_instanceStrId("SYSTEMID_WS00_TUN"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_REQUEST)
{
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
}

void Settings::StoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("MainWindow/Splitter/state", m_mainWindowSplitterState);

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

	s.setValue("Filters/filterByEquipment", m_filterByEquipment);
	s.setValue("Filters/filterBySchema", m_filterBySchema);

	s.setValue("PropertyEditor/multiLinePos", m_multiLinePropertyEditorWindowPos);
	s.setValue("PropertyEditor/multiLineGeometry", m_multiLinePropertyEditorGeometry);

	s.setValue("PresetProperties/splitterState", m_presetPropertiesSplitterState);
	s.setValue("PresetProperties/pos", m_presetPropertiesWindowPos);
	s.setValue("PresetProperties/geometry", m_presetPropertiesWindowGeometry);

}

void Settings::RestoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_mainWindowSplitterState = s.value("MainWindow/Splitter/state").toByteArray();

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

	m_filterByEquipment = s.value("Filters/filterByEquipment", m_filterByEquipment).toBool();
	m_filterBySchema = s.value("Filters/filterBySchema", m_filterBySchema).toBool();

	m_multiLinePropertyEditorWindowPos = s.value("PropertyEditor/multiLinePos", QPoint(-1, -1)).toPoint();
	m_multiLinePropertyEditorGeometry = s.value("PropertyEditor/multiLineGeometry").toByteArray();

	m_presetPropertiesSplitterState = s.value("PresetProperties/splitterState").toInt();
	if (m_presetPropertiesSplitterState < 100)
		m_presetPropertiesSplitterState = 100;
	m_presetPropertiesWindowPos = s.value("PresetProperties/pos", QPoint(-1, -1)).toPoint();
	m_presetPropertiesWindowGeometry = s.value("PresetProperties/geometry").toByteArray();
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

bool Settings::filterByEquipment() const
{
	return m_filterByEquipment;
}

void Settings::setFilterByEquipment(bool value)
{
	m_filterByEquipment = value;
}

bool Settings::filterBySchema() const
{
	return m_filterBySchema;
}

void Settings::setFilterBySchema(bool value)
{
	m_filterBySchema = value;
}

bool Settings::admin() const
{
	return m_admin;
}

Settings theSettings;
