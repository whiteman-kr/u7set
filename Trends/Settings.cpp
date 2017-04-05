#include "Settings.h"

Settings theSettings;

Settings::Settings()
{
}

Settings::~Settings()
{
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
	//QSettings s;
	//s.setValue("m_instanceStrId", m_instanceStrId);
	return;
}

void Settings::loadSystemScope()
{
	//QSettings s;
	//m_instanceStrId = s.value("m_instanceStrId", "SYSTEM_RACKID_WS00_MONITOR").toString();
	return;
}
