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

void Settings::StoreUser()
{
	QSettings s;

	QMutexLocker l(&m);

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("MainWindow/Splitter/state", m_mainWindowSplitterState);

}

void Settings::RestoreUser()
{
	QSettings s;

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_mainWindowSplitterState = s.value("MainWindow/Splitter/state").toByteArray();
}

QString Settings::instanceStrId()
{
	QMutexLocker l(&m);
	return m_instanceStrId;
}

HostAddressPort Settings::configuratorAddress1()
{
	QMutexLocker l(&m);
	return HostAddressPort(m_configuratorIpAddress1, m_configuratorPort1);
}

HostAddressPort Settings::configuratorAddress2()
{
	QMutexLocker l(&m);
	return HostAddressPort(m_configuratorIpAddress2, m_configuratorPort2);
}

Settings theSettings;
