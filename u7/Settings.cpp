#include "Settings.h"
#include "QStandardPaths"

Settings theSettings;

Settings::Settings() :
	m_serverIpAddress("127.0.0.1"),
	m_serverPort(0),
	m_serverUsername("u7"),
	m_serverPassword("P2ssw0rd")
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

	s.setValue("loginDialog_defaultUsername", loginDialog_defaultUsername);

	s.setValue("ConfigurationTabPage/Splitter/state", m_configurationTabPageSplitterState);

	s.setValue("EquipmentTabPage/Splitter/state", m_equipmentTabPageSplitterState);

	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	loginDialog_defaultUsername = s.value("loginDialog_defaultUsername").toString();

	m_configurationTabPageSplitterState = s.value("ConfigurationTabPage/Splitter/state").toByteArray();

	m_equipmentTabPageSplitterState = s.value("EquipmentTabPage/Splitter/state").toByteArray();

	return;
}

void Settings::writeSystemScope() const
{
	QSettings s;

	s.setValue("m_serverIpAddress", m_serverIpAddress);
	s.setValue("m_serverPort", m_serverPort);
	s.setValue("m_serverUsername", m_serverUsername);
	s.setValue("m_serverPassword", m_serverPassword);

	return;
}
void Settings::loadSystemScope()
{
	QSettings s;

	m_serverIpAddress = s.value("m_serverIpAddress", "127.0.0.1").toString();
	m_serverPort = s.value("m_serverPort", 5432).toInt();
	m_serverUsername = s.value("m_serverUsername", "u7").toString();
	m_serverPassword = s.value("m_serverPassword", "P2ssw0rd").toString();

	return;
}

const QString& Settings::serverIpAddress() const
{
	return m_serverIpAddress;
}
void Settings::setServerIpAddress(const QString& value)
{
	m_serverIpAddress = value;
}

int Settings::serverPort() const
{
	return m_serverPort;
}
void Settings::setServerPort(int value)
{
	m_serverPort = value;
}

const QString& Settings::serverUsername() const
{
	return m_serverUsername;
}
void Settings::setServerUsername(const QString& value)
{
	m_serverUsername = value;
}

const QString& Settings::serverPassword() const
{
	return m_serverPassword;
}
void Settings::setServerPassword(const QString& value)
{
	m_serverPassword = value;
}

