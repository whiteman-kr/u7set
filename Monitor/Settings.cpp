#include "Settings.h"
#include "QStandardPaths"
#include "QDir"
#include "../include/SocketIO.h"

Settings theSettings;

Settings::Settings() :
	m_configuratorIpAddress("127.0.0.1"),
	m_configuratorPort(PORT_CONFIG_SERVICE)
{
}

Settings::~Settings()
{
}

void Settings::write() const
{
	writeUserScope();
	writeSystemScope();
}

void Settings::load()
{
	loadUserScope();
	loadSystemScope();
}

void Settings::writeUserScope() const
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

    return;
}

void Settings::writeSystemScope() const
{
	QSettings s;

	s.setValue("m_configuratorIpAddress", m_configuratorIpAddress);
	s.setValue("m_configuratorPort", m_configuratorPort);

	return;
}
void Settings::loadSystemScope()
{
	QSettings s;

	m_configuratorIpAddress = s.value("m_configuratorIpAddress", "127.0.0.1").toString();
	m_configuratorPort = s.value("m_configuratorPort", PORT_CONFIG_SERVICE).toInt();

	return;
}

QString Settings::configuratorIpAddress() const
{
	return m_configuratorIpAddress;
}

void Settings::setConfiguratorIpAddress(QString configuratorIpAddress)
{
	m_configuratorIpAddress = configuratorIpAddress;
}

int Settings::configuratorPort() const
{
	return m_configuratorPort;
}

void Settings::setConfiguratorPort(int configuratorPort)
{
	m_configuratorPort = configuratorPort;
}


