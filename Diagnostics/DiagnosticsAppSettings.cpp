#include "DiagnosticsAppSettings.h"

#include "../lib/ConstStrings.h"
#include "../OnlineLib/SocketIO.h"

DiagnosticsAppSettings& DiagnosticsAppSettings::instance()
{
	static DiagnosticsAppSettings theSettings;
	return theSettings;
}

void DiagnosticsAppSettings::save() const
{
	QSettings s{Manufacturer::RADIY, "Diagnostics3"};	// Explicitly point app name, as it can be changed via settings.
	save(s);
	return;
}

void DiagnosticsAppSettings::restore()
{
	QSettings s{Manufacturer::RADIY, "Diagnostics3"};	// Explicitly point app name, as it can be changed via settings.
	load(s);
	m_wasLoadedFromFile = false;
	return;
}

bool DiagnosticsAppSettings::saveToFile(QString fileName) const
{
	QSettings s{fileName, QSettings::IniFormat};
	save(s);
	s.sync();
	return s.status() == QSettings::Status::NoError;
}

bool DiagnosticsAppSettings::loadFromFile(QString fileName)
{
	QSettings s{fileName, QSettings::IniFormat};
	load(s);
	m_wasLoadedFromFile = true;
	return s.status() == QSettings::Status::NoError;
}

bool DiagnosticsAppSettings::wasLoadedFromFile() const
{
	return m_wasLoadedFromFile;
}

void DiagnosticsAppSettings::save(QSettings& settings) const
{
	auto data = get();

	settings.setValue("DiagnosticsAppSettings/equipmentId", data.equipmentId);
	settings.setValue("DiagnosticsAppSettings/windowCaption", data.windowCaption);
	settings.setValue("DiagnosticsAppSettings/language", data.language);

	settings.setValue("DiagnosticsAppSettings/configuratorIpAddress1", data.cfgSrvIpAddress1);
	settings.setValue("DiagnosticsAppSettings/configuratorPort1", data.cfgSrvPort1);

	settings.setValue("DiagnosticsAppSettings/configuratorIpAddress2", data.cfgSrvIpAddress2);
	settings.setValue("DiagnosticsAppSettings/configuratorPort2", data.cfgSrvPort2);

	settings.setValue("DiagnosticsAppSettings/showSchemasTabBar", data.showSchemasTabBar);
	settings.setValue("DiagnosticsAppSettings/showLogo", data.showLogo);
	settings.setValue("DiagnosticsAppSettings/showItemsLabels", data.showItemsLabels);
	settings.setValue("DiagnosticsAppSettings/singleInstance", data.singleInstance);
	settings.setValue("DiagnosticsAppSettings/zoomMode", static_cast<int>(data.zoomMode));

	return;
}

void DiagnosticsAppSettings::load(const QSettings& settings)
{
	Data data;

	data.equipmentId = settings.value("DiagnosticsAppSettings/equipmentId", "SYSTEM_RACKID_WS00_DIAGNOSTICS").toString();
	data.windowCaption = settings.value("DiagnosticsAppSettings/windowCaption", "Diagnostics").toString();
	data.language = settings.value("DiagnosticsAppSettings/language", "en").toString();

	data.cfgSrvIpAddress1 = settings.value("DiagnosticsAppSettings/configuratorIpAddress1", "127.0.0.1").toString();
	data.cfgSrvPort1 = settings.value("DiagnosticsAppSettings/configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	data.cfgSrvIpAddress2 = settings.value("DiagnosticsAppSettings/configuratorIpAddress2", "127.0.0.1").toString();
	data.cfgSrvPort2 = settings.value("DiagnosticsAppSettings/configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	data.showSchemasTabBar = settings.value("DiagnosticsAppSettings/showSchemasTabBar", true).toBool();
	data.showLogo = settings.value("DiagnosticsAppSettings/showLogo", true).toBool();
	data.showItemsLabels = settings.value("DiagnosticsAppSettings/showItemsLabels", false).toBool();
	data.singleInstance = settings.value("DiagnosticsAppSettings/singleInstance", false).toBool();

	data.zoomMode = static_cast<VFrame30::ZoomMode>(settings.value("DiagnosticsAppSettings/zoomMode", 
																		 static_cast<int>(VFrame30::ZoomMode::Manual)).toInt());

	set(data);
}

DiagnosticsAppSettings::Data DiagnosticsAppSettings::get() const
{
	QMutexLocker l(&m_mutex);
	return m_data;
}

void DiagnosticsAppSettings::set(const DiagnosticsAppSettings::Data& src)
{
	QMutexLocker l(&m_mutex);
	m_data = src;
}


QString DiagnosticsAppSettings::equipmentId() const
{
	QMutexLocker l(&m_mutex);
	return m_data.equipmentId;
}

QString DiagnosticsAppSettings::windowCaption() const
{
	QMutexLocker l(&m_mutex);
	return m_data.windowCaption;
}

QString DiagnosticsAppSettings::language() const
{
	QMutexLocker l(&m_mutex);
	return m_data.language;
}

HostAddressPort DiagnosticsAppSettings::configuratorAddress1() const
{
	QMutexLocker l(&m_mutex);
	HostAddressPort result{m_data.cfgSrvIpAddress1, static_cast<quint16>(m_data.cfgSrvPort1)};
	return result;
}

HostAddressPort DiagnosticsAppSettings::configuratorAddress2() const
{
	QMutexLocker l(&m_mutex);
	HostAddressPort result{m_data.cfgSrvIpAddress2, static_cast<quint16>(m_data.cfgSrvPort2)};
	return result;
}

QString DiagnosticsAppSettings::configuratorIpAddress1() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvIpAddress1;
}

int DiagnosticsAppSettings::configuratorPort1() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvPort1;
}

QString DiagnosticsAppSettings::configuratorIpAddress2() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvIpAddress2;
}

int DiagnosticsAppSettings::configuratorPort2() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvPort2;
}

int DiagnosticsAppSettings::requestTimeInterval() const
{
	QMutexLocker l(&m_mutex);
	return m_data.requestTimeIntervalMs;
}

bool DiagnosticsAppSettings::showSchemasTabBar() const
{
	QMutexLocker l(&m_mutex);
	return m_data.showSchemasTabBar;
}

bool DiagnosticsAppSettings::showLogo() const
{
	QMutexLocker l(&m_mutex);
	return m_data.showLogo;
}

bool DiagnosticsAppSettings::showItemsLabels() const
{
	QMutexLocker l(&m_mutex);
	return m_data.showItemsLabels;
}

VFrame30::ZoomMode DiagnosticsAppSettings::zoomMode() const
{
	QMutexLocker l(&m_mutex);
	return m_data.zoomMode;
}

bool DiagnosticsAppSettings::singleInstance() const
{
	QMutexLocker l(&m_mutex);
	return m_data.singleInstance;
}
