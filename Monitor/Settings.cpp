#include "Settings.h"
#include "../lib/SocketIO.h"

Settings theSettings;

Settings::Settings() :
	m_instanceStrId("SYSTEM_RACKID_WS00_MONITOR"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST)
{
	 qRegisterMetaTypeStreamOperators<QList<int>>("QList<int>");
}

Settings::~Settings()
{
}

Settings& Settings::operator = (const Settings& src)
{
	if (this != &src)
	{
		setInstanceStrId(src.instanceStrId());

		setConfiguratorIpAddress1(src.configuratorIpAddress1());
		setConfiguratorPort1(src.configuratorPort1());

		setConfiguratorIpAddress2(src.configuratorIpAddress2());
		setConfiguratorPort2(src.configuratorPort2());

		setShowLogo(src.showLogo());
		setShowItemsLabels(src.showItemsLabels());
		setSingleInstance(src.singleInstance());
	}

	return *this;
}

void Settings::write() const
{
	QMutexLocker l(&m_mutex);

	writeUserScope();
	writeSystemScope();
}

void Settings::load()
{
	QMutexLocker l(&m_mutex);

	loadUserScope();
	loadSystemScope();
}

void Settings::writeUserScope() const
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("DialogChooseTrendSignals/filter", m_trendSignalsDialogFilterCompleter);

	s.setValue("ArchiveWindow/pos", m_archiveWindowPos);
	s.setValue("ArchiveWindow/geometry", m_archiveWindowGeometry);
	s.setValue("ArchiveWindow/state", m_archiveWindowState);
	s.setValue("ArchiveWindow/horzHeader", m_archiveHorzHeader);
	s.setValue("ArchiveWindow/horzHeaderCount", m_archiveHorzHeaderCount);

	s.setValue("ArchiveWindow/timeType", m_archiveTimeType);

	s.setValue("DialogChooseArchiveSignals/filter", m_archiveSignalsDialogFilterCompleter);


	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_trendSignalsDialogFilterCompleter = s.value("DialogChooseTrendSignals/filter").toStringList();

	m_archiveWindowPos = s.value("ArchiveWindow/pos", QPoint(200, 200)).toPoint();
	m_archiveWindowGeometry = s.value("ArchiveWindow/geometry").toByteArray();
	m_archiveWindowState = s.value("ArchiveWindow/state").toByteArray();
	m_archiveHorzHeader = s.value("ArchiveWindow/horzHeader").toByteArray();
	m_archiveHorzHeaderCount = s.value("ArchiveWindow/horzHeaderCount").toInt();
	m_archiveTimeType = s.value("ArchiveWindow/timeType").toInt();
	m_archiveSignalsDialogFilterCompleter = s.value("DialogChooseArchiveSignals/filter").toStringList();

	return;
}

void Settings::writeSystemScope() const
{
	QSettings s;

	s.setValue("m_instanceStrId", m_instanceStrId);

	s.setValue("m_configuratorIpAddress1", m_configuratorIpAddress1);
	s.setValue("m_configuratorPort1", m_configuratorPort1);

	s.setValue("m_configuratorIpAddress2", m_configuratorIpAddress2);
	s.setValue("m_configuratorPort2", m_configuratorPort2);

	s.setValue("m_showLogo", m_showLogo);
	s.setValue("m_showItemsLabels", m_showItemsLabels);
	s.setValue("m_singleInstance", m_singleInstance);

	return;
}
void Settings::loadSystemScope()
{
	QSettings s;

	m_instanceStrId = s.value("m_instanceStrId", "SYSTEM_RACKID_WS00_MONITOR").toString();

	m_configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
	m_configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
	m_configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_showLogo = s.value("m_showLogo", true).toBool();
	m_showItemsLabels = s.value("m_showItemsLabels", false).toBool();
	m_singleInstance = s.value("m_singleInstance", false).toBool();

	return;
}

QString Settings::instanceStrId() const
{
	QMutexLocker l(&m_mutex);
	return m_instanceStrId;
}

void Settings::setInstanceStrId(QString value)
{
	QMutexLocker l(&m_mutex);
	m_instanceStrId = value;
}


HostAddressPort Settings::configuratorAddress1() const
{
	QMutexLocker l(&m_mutex);

	HostAddressPort result{m_configuratorIpAddress1, static_cast<quint16>(m_configuratorPort1)};

	return result;
}

HostAddressPort Settings::configuratorAddress2() const
{
	QMutexLocker l(&m_mutex);

	HostAddressPort result{m_configuratorIpAddress2, static_cast<quint16>(m_configuratorPort2)};

	return result;
}

QString Settings::configuratorIpAddress1() const
{
	QMutexLocker l(&m_mutex);
	return m_configuratorIpAddress1;
}

void Settings::setConfiguratorIpAddress1(QString configuratorIpAddress)
{
	QMutexLocker l(&m_mutex);
	m_configuratorIpAddress1 = configuratorIpAddress;
}

int Settings::configuratorPort1() const
{
	QMutexLocker l(&m_mutex);
	return m_configuratorPort1;
}

void Settings::setConfiguratorPort1(int configuratorPort)
{
	QMutexLocker l(&m_mutex);
	m_configuratorPort1 = configuratorPort;
}

QString Settings::configuratorIpAddress2() const
{
	QMutexLocker l(&m_mutex);
	return m_configuratorIpAddress2;
}

void Settings::setConfiguratorIpAddress2(QString configuratorIpAddress)
{
	QMutexLocker l(&m_mutex);
	m_configuratorIpAddress2 = configuratorIpAddress;
}

int Settings::configuratorPort2() const
{
	QMutexLocker l(&m_mutex);
	return m_configuratorPort2;
}

void Settings::setConfiguratorPort2(int configuratorPort)
{
	QMutexLocker l(&m_mutex);
	m_configuratorPort2 = configuratorPort;
}

int Settings::requestTimeInterval() const
{
	QMutexLocker l(&m_mutex);
	return m_requestTimeInterval;
}

void Settings::setRequestTimeInterval(int value)
{
	QMutexLocker l(&m_mutex);
	m_requestTimeInterval = value;
}

bool Settings::showLogo() const
{
	QMutexLocker l(&m_mutex);
	return m_showLogo;
}

void Settings::setShowLogo(bool value)
{
	QMutexLocker l(&m_mutex);
	m_showLogo = value;
}

bool Settings::showItemsLabels() const
{
	QMutexLocker l(&m_mutex);
	return m_showItemsLabels;
}

void Settings::setShowItemsLabels(bool value)
{
	QMutexLocker l(&m_mutex);
	m_showItemsLabels = value;
}

bool Settings::singleInstance() const
{
	QMutexLocker l(&m_mutex);
	return m_singleInstance;
}

void Settings::setSingleInstance(bool value)
{
	QMutexLocker l(&m_mutex);
	m_singleInstance = value;
}
