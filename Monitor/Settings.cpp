#include "Settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include "../lib/SocketIO.h"

Settings theSettings;

Settings::Settings() :
	m_instanceStrId("SYSTEM_RACKID_WS00_MONITOR"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_REQUEST)
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

	s.setValue("DialogSignalSearch/pos", m_signalSearchPos);
	s.setValue("DialogSignalSearch/geometry", m_signalSearchGeometry);
	s.setValue("DialogSignalSearch/columnCount", m_signalSearchColumnCount);
	s.setValue("DialogSignalSearch/columnWidth", m_signalSearchColumnWidth);

	s.setValue("DialogSignalSnapshot/pos", m_signalSnapshotPos);
	s.setValue("DialogSignalSnapshot/geometry", m_signalSnapshotGeometry);
	s.setValue("DialogSignalSnapshot/columns", QVariant::fromValue<QList<int>>(m_signalSnapshotColumns.toList()));
	s.setValue("DialogSignalSnapshot/type", m_signalSnapshotSignalType);
	s.setValue("DialogSignalSnapshot/mask", m_signalSnapshotMaskList);

	s.setValue("DialogSignalSnapshot/maskType", static_cast<int>(m_signalSnapshotMaskType));
	s.setValue("DialogSignalSnapshot/sortColumn", m_signalSnapshotSortColumn);
	s.setValue("DialogSignalSnapshot/sortOrder", static_cast<int>(m_signalSnapshotSortOrder));

	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_signalSearchPos = s.value("DialogSignalSearch/pos", QPoint(-1, -1)).toPoint();
	m_signalSearchGeometry = s.value("DialogSignalSearch/geometry").toByteArray();
	m_signalSearchColumnCount = s.value("DialogSignalSearch/columnCount").toInt();
	m_signalSearchColumnWidth = s.value("DialogSignalSearch/columnWidth").toByteArray();

	m_signalSnapshotPos = s.value("DialogSignalSnapshot/pos", QPoint(-1, -1)).toPoint();
	m_signalSnapshotGeometry = s.value("DialogSignalSnapshot/geometry").toByteArray();
	m_signalSnapshotColumns = s.value("DialogSignalSnapshot/columns").value<QList<int>>().toVector();
	m_signalSnapshotSignalType = s.value("DialogSignalSnapshot/type").toInt();
	m_signalSnapshotMaskList = s.value("DialogSignalSnapshot/mask").toStringList();
	m_signalSnapshotMaskType = static_cast<DialogSignalSnapshot::MaskType>(s.value("DialogSignalSnapshot/maskType", static_cast<int>(m_signalSnapshotMaskType)).toInt());
	m_signalSnapshotSortColumn = s.value("DialogSignalSnapshot/sortColumn", m_signalSnapshotSortColumn).toInt();
	m_signalSnapshotSortOrder = static_cast<Qt::SortOrder>(s.value("DialogSignalSnapshot/sortOrder", m_signalSnapshotSortOrder).toInt());

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

	return;
}
void Settings::loadSystemScope()
{
	QSettings s;

	m_instanceStrId = s.value("m_instanceStrId", "SYSTEM_RACKID_WS00_MONITOR").toString();

	m_configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
	m_configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_REQUEST).toInt();

	m_configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
	m_configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_REQUEST).toInt();

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

	HostAddressPort result;
	result.setAddress(m_configuratorIpAddress1);
	result.setPort(m_configuratorPort1);

	return result;
}

HostAddressPort Settings::configuratorAddress2() const
{
	QMutexLocker l(&m_mutex);

	HostAddressPort result;
	result.setAddress(m_configuratorIpAddress2);
	result.setPort(m_configuratorPort2);

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
