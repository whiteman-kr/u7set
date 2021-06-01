#include "MonitorAppSettings.h"
#include "../OnlineLib/SocketIO.h"


MonitorAppSettings::MonitorAppSettings()
{
	 qRegisterMetaTypeStreamOperators<QList<int>>("QList<int>");
}

MonitorAppSettings& MonitorAppSettings::instance()
{
	static MonitorAppSettings theSettings;
	return theSettings;
}

void MonitorAppSettings::save() const
{
	QSettings s{};
	save(s);
	return;
}

void MonitorAppSettings::restore()
{
	QSettings s{};
	load(s);
	return;
}

bool MonitorAppSettings::saveToFile(QString fileName) const
{
	QSettings s{fileName, QSettings::IniFormat};
	save(s);
	s.sync();
	return s.status() == QSettings::Status::NoError;
}

bool MonitorAppSettings::loadFromFile(QString fileName)
{
	QSettings s{fileName, QSettings::IniFormat};
	load(s);
	return s.status() == QSettings::Status::NoError;
}

void MonitorAppSettings::save(QSettings& settings) const
{
	auto data = get();

	settings.setValue("MonitorAppSettings/equipmentId", data.equipmentId);
	settings.setValue("MonitorAppSettings/windowCaption", data.windowCaption);

	settings.setValue("MonitorAppSettings/configuratorIpAddress1", data.cfgSrvIpAddress1);
	settings.setValue("MonitorAppSettings/configuratorPort1", data.cfgSrvPort1);

	settings.setValue("MonitorAppSettings/configuratorIpAddress2", data.cfgSrvIpAddress2);
	settings.setValue("MonitorAppSettings/configuratorPort2", data.cfgSrvPort2);

	settings.setValue("MonitorAppSettings/showLogo", data.showLogo);
	settings.setValue("MonitorAppSettings/showItemsLabels", data.showItemsLabels);
	settings.setValue("MonitorAppSettings/singleInstance", data.singleInstance);

	return;
}

void MonitorAppSettings::load(const QSettings& settings)
{
	Data data;

	data.equipmentId = settings.value("MonitorAppSettings/equipmentId", "SYSTEM_RACKID_WS00_MONITOR").toString();
	data.windowCaption = settings.value("MonitorAppSettings/windowCaption", "Monitor").toString();

	data.cfgSrvIpAddress1 = settings.value("MonitorAppSettings/configuratorIpAddress1", "127.0.0.1").toString();
	data.cfgSrvPort1 = settings.value("MonitorAppSettings/configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	data.cfgSrvIpAddress2 = settings.value("MonitorAppSettings/configuratorIpAddress2", "127.0.0.1").toString();
	data.cfgSrvPort2 = settings.value("MonitorAppSettings/configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	data.showLogo = settings.value("MonitorAppSettings/showLogo", true).toBool();
	data.showItemsLabels = settings.value("MonitorAppSettings/showItemsLabels", false).toBool();
	data.singleInstance = settings.value("MonitorAppSettings/singleInstance", false).toBool();

	set(data);
}

MonitorAppSettings::Data MonitorAppSettings::get() const
{
	QMutexLocker l(&m_mutex);
	return m_data;
}

void MonitorAppSettings::set(const MonitorAppSettings::Data& src)
{
	QMutexLocker l(&m_mutex);
	m_data = src;
}


QString MonitorAppSettings::equipmentId() const
{
	QMutexLocker l(&m_mutex);
	return m_data.equipmentId;
}

QString MonitorAppSettings::windowCaption() const
{
	QMutexLocker l(&m_mutex);
	return m_data.windowCaption;
}

HostAddressPort MonitorAppSettings::configuratorAddress1() const
{
	QMutexLocker l(&m_mutex);
	HostAddressPort result{m_data.cfgSrvIpAddress1, static_cast<quint16>(m_data.cfgSrvPort1)};
	return result;
}

HostAddressPort MonitorAppSettings::configuratorAddress2() const
{
	QMutexLocker l(&m_mutex);
	HostAddressPort result{m_data.cfgSrvIpAddress2, static_cast<quint16>(m_data.cfgSrvPort2)};
	return result;
}

QString MonitorAppSettings::configuratorIpAddress1() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvIpAddress1;
}

int MonitorAppSettings::configuratorPort1() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvPort1;
}

QString MonitorAppSettings::configuratorIpAddress2() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvIpAddress2;
}

int MonitorAppSettings::configuratorPort2() const
{
	QMutexLocker l(&m_mutex);
	return m_data.cfgSrvPort2;
}

int MonitorAppSettings::requestTimeInterval() const
{
	QMutexLocker l(&m_mutex);
	return m_data.requestTimeIntervalMs;
}

bool MonitorAppSettings::showLogo() const
{
	QMutexLocker l(&m_mutex);
	return m_data.showLogo;
}

bool MonitorAppSettings::showItemsLabels() const
{
	QMutexLocker l(&m_mutex);
	return m_data.showItemsLabels;
}

bool MonitorAppSettings::singleInstance() const
{
	QMutexLocker l(&m_mutex);
	return m_data.singleInstance;
}
