#include "TrendSettings.h"

namespace TrendLib
{

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

		s.setValue("Trends/MainWindow/pos", m_mainWindowPos);
		s.setValue("Trends/MainWindow/geometry", m_mainWindowGeometry);
		s.setValue("Trends/MainWindow/state", m_mainWindowState);
		s.setValue("Trends/MainWindow/ToolBar/viewType", m_viewType);
		s.setValue("Trends/MainWindow/ToolBar/laneCount", m_laneCount);
		s.setValue("Trends/MainWindow/ToolBar/timeTypeIndex", m_timeTypeIndex);
		s.setValue("Trends/MainWindow/ToolBar/timeType", m_timeType);

		return;
	}

	void Settings::loadUserScope()
	{
		QSettings s;

		m_mainWindowPos = s.value("Trends/MainWindow/pos", QPoint(200, 200)).toPoint();
		m_mainWindowGeometry = s.value("Trends/MainWindow/geometry").toByteArray();
		m_mainWindowState = s.value("Trends/MainWindow/state").toByteArray();
		m_viewType = s.value("Trends/MainWindow/ToolBar/viewType", 0).toInt();
		m_laneCount = s.value("Trends/MainWindow/ToolBar/laneCount", 2).toInt();
		m_timeTypeIndex = s.value("Trends/MainWindow/ToolBar/timeTypeIndex", 0).toInt();
		m_timeType = s.value("Trends/MainWindow/ToolBar/timeType", 0).toInt();

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

}
