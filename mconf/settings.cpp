#include "Stable.h"
#include "settings.h"


Settings::Settings(void) :
	m_serialPort("\\\\.\\COM3"),
	m_showDebugInfo(false),
	m_expertMode(false),
	m_server("127.0.0.1"),
	m_serverUsername("u7"),
	m_serverPassword("P2ssw0rd")
{
}


Settings::~Settings(void)
{
}

void Settings::save() const
{
	QSettings s;
	s.setValue("m_serialPort", m_serialPort);
	s.setValue("m_showDebugInfo", m_showDebugInfo);
	s.setValue("m_expertMode", m_expertMode);

	s.setValue("m_server", m_server);
	s.setValue("m_serverUsername", m_serverUsername);
	s.setValue("m_serverPassword", m_serverPassword);

	return;
}

void Settings::load()
{
	QSettings s;
	m_serialPort = s.value("m_serialPort").toString();
	m_showDebugInfo = s.value("m_showDebugInfo").toBool();
	m_expertMode = s.value("m_expertMode").toBool();

	m_server = s.value("m_server", "127.0.0.1").toString();
	m_serverUsername = s.value("m_serverUsername", "u7").toString();
	m_serverPassword = s.value("m_serverPassword", "P2ssw0rd").toString();

	return;
}

QString Settings::serialPort() const
{
	return m_serialPort;
}

void Settings::setSerialPort(const QString& value)
{
	m_serialPort = value;
}

bool Settings::showDebugInfo() const
{
	return m_showDebugInfo;
}

void Settings::setShowDebugInfo(bool value)
{
	m_showDebugInfo = value;
}

bool Settings::expertMode() const
{
	return m_expertMode;
}

void Settings::setExpertMode(bool value)
{
	m_expertMode = value;
}

QString Settings::server() const
{
	return m_server;
}

void Settings::setServer(const QString& value)
{
	m_server = value;
}

QString Settings::serverUsername() const
{
	return m_serverUsername;
}

void Settings::setServerUsername(const QString& value)
{
	m_serverUsername = value;
}

QString Settings::serverPassword() const
{
	return m_serverPassword;
}

void Settings::setServerPassword(const QString& value)
{
	m_serverPassword = value;
}
