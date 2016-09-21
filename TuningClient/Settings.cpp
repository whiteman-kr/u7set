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

	s.setValue("TuningPageSettingsCount", static_cast<uint>(m_tuningPageSettings.size()));
	for (int i = 0; i < m_tuningPageSettings.size(); i++)
	{
		s.setValue(QString("TuningPageSettings%1/columnCount").arg(i), m_tuningPageSettings[i].m_columnCount);
		for (int c = 0; c < m_tuningPageSettings[i].m_columnCount; c++)
		{
			s.setValue(QString("TuningPageSettings%1/columnWidth/%2").arg(i).arg(c), m_tuningPageSettings[i].m_columnsWidth[c]);
			s.setValue(QString("TuningPageSettings%1/columnIndex/%2").arg(i).arg(c), m_tuningPageSettings[i].m_columnsIndexes[c]);
		}
	}

}

void Settings::RestoreUser()
{
	QSettings s;

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_mainWindowSplitterState = s.value("MainWindow/Splitter/state").toByteArray();

	int tuningPageSettingsCount = s.value("TuningPageSettingsCount", 0).toInt();
	m_tuningPageSettings.resize(tuningPageSettingsCount);

	for (int i = 0; i < tuningPageSettingsCount; i++)
	{
		m_tuningPageSettings[i].m_columnCount = s.value(QString("TuningPageSettings%1/columnCount").arg(i), 0).toInt();
		m_tuningPageSettings[i].m_columnsWidth.resize(m_tuningPageSettings[i].m_columnCount);
		m_tuningPageSettings[i].m_columnsIndexes.resize(m_tuningPageSettings[i].m_columnCount);
		for (int c = 0; c < m_tuningPageSettings[i].m_columnCount; c++)
		{
			m_tuningPageSettings[i].m_columnsWidth[c] = s.value(QString("TuningPageSettings%1/columnWidth/%2").arg(i).arg(c), 100).toInt();
			m_tuningPageSettings[i].m_columnsIndexes[c] = s.value(QString("TuningPageSettings%1/columnIndex/%2").arg(i).arg(c), 0).toInt();
		}
	}
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
