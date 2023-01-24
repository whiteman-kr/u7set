#include "Settings.h"
#include "../OnlineLib/SocketIO.h"

QColor redColor = QColor(192, 0, 0);

//
// Settings
//

Settings::Settings():
	m_instanceStrId("SYSTEMID_WS00_TESTSUITE"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
	m_language("en")
{
}

Settings::Settings(const Settings& That):
	Settings()
{
	*this = That;
}

void Settings::StoreSystem()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QString instanceHistoryString = m_instanceHistory.join(';');
	s.setValue("m_instanceHistory", instanceHistoryString);

	s.setValue("m_instanceStrId", m_instanceStrId);

	s.setValue("m_configuratorIpAddress1", m_configuratorIpAddress1);
	s.setValue("m_configuratorPort1", m_configuratorPort1);

	s.setValue("m_configuratorIpAddress2", m_configuratorIpAddress2);
	s.setValue("m_configuratorPort2", m_configuratorPort2);
}

void Settings::RestoreSystem()
{
	// read system settings
	//
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QString instanceHistoryString = s.value("m_instanceHistory", QString()).toString();
	m_instanceHistory = instanceHistoryString.split(';', Qt::SkipEmptyParts);

	m_instanceStrId = s.value("m_instanceStrId", m_instanceStrId).toString();

	m_configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
	m_configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
	m_configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	// Determine the Local settings folder

	m_localAppDataPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	QDir dir(m_localAppDataPath);

	if (dir.exists() == false)
	{
		dir.mkpath(m_localAppDataPath);
	}

}


void Settings::StoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("MainWindow/language", m_language);

}

void Settings::RestoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_language = s.value("MainWindow/language", m_language).toString();
}

QStringList Settings::instanceHistory() const
{
	QMutexLocker l(&m);
	return m_instanceHistory;
}

void Settings::setInstanceHistory(const QStringList& value)
{
	QMutexLocker l(&m);
	m_instanceHistory = value;
}

QString Settings::instanceStrId() const
{
	QMutexLocker l(&m);
	return m_instanceStrId;
}

void Settings::setInstanceStrId(const QString& value)
{
	QMutexLocker l(&m);
	m_instanceStrId = value;
}

HostAddressPort Settings::configuratorAddress1() const
{
	QMutexLocker l(&m);
	return HostAddressPort(m_configuratorIpAddress1, m_configuratorPort1);
}

void Settings::setConfiguratorAddress1(const QString& address, int port)
{
	m_configuratorIpAddress1 = address;
	m_configuratorPort1 = port;
}

HostAddressPort Settings::configuratorAddress2() const
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

QString Settings::localAppDataPath()
{
	return m_localAppDataPath;
}

Settings theSettings;

