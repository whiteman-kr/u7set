#include "Settings.h"

Settings::Settings()
{

}

void Settings::StoreUser()
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	qDebug()<<m_mainWindowSplitterState.size();

	s.setValue("MainWindow/Splitter/state", m_mainWindowSplitterState);

}

void Settings::RestoreUser()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_mainWindowSplitterState = s.value("MainWindow/Splitter/state").toByteArray();

}


Settings theSettings;
