#include "Options.h"

#include <QSettings>

#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BuildOption::BuildOption(QObject *parent) :
	QObject(parent)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption::BuildOption(const BuildOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption::~BuildOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::clear()
{
	m_buildInfo.clear();
	m_signalsStatePath.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::load()
{
	QSettings s;

	m_buildInfo.setBuildDirPath(s.value(QString("%1BuildDirPath").arg(BUILD_REG_KEY), QString()).toString());

	m_buildInfo.setEnableReload(s.value(QString("%1EnableReloadBuildFiles").arg(BUILD_REG_KEY), true).toBool());
	m_buildInfo.setTimeoutReload(s.value(QString("%1TimeoutReloadBuildFiles").arg(BUILD_REG_KEY), BUILD_INFO_RELOAD_TIMEOUT).toInt());

	m_buildInfo.setAppDataSrvIP(	HostAddressPort(	s.value(QString("%1AppDataSrvIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString(),
														s.value(QString("%1AppDataSrvPort").arg(BUILD_REG_KEY), PORT_APP_DATA_SERVICE_DATA).toInt()));

	m_buildInfo.setUalTesterIP(		HostAddressPort(	s.value(QString("%1UalTesterIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString(),
														s.value(QString("%1UalTesterPort").arg(BUILD_REG_KEY), PORT_TUNING_SERVICE_CLIENT_REQUEST).toInt()));

	m_signalsStatePath = s.value(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), QString("SignalStates.csv")).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::save()
{
	QSettings s;

	s.setValue(QString("%1BuildDirPath").arg(BUILD_REG_KEY), m_buildInfo.buildDirPath());

	s.setValue(QString("%1EnableReloadBuildFiles").arg(BUILD_REG_KEY), m_buildInfo.enableReload());
	s.setValue(QString("%1TimeoutReloadBuildFiles").arg(BUILD_REG_KEY), m_buildInfo.timeoutReload());

	s.setValue(QString("%1AppDataSrvIP").arg(BUILD_REG_KEY), m_buildInfo.appDataSrvIP().address().toString());
	s.setValue(QString("%1AppDataSrvPort").arg(BUILD_REG_KEY), m_buildInfo.appDataSrvIP().port());

	s.setValue(QString("%1UalTesterIP").arg(BUILD_REG_KEY), m_buildInfo.ualTesterIP().address().toString());
	s.setValue(QString("%1UalTesterPort").arg(BUILD_REG_KEY), m_buildInfo.ualTesterIP().port());

	s.setValue(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), m_signalsStatePath);
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption& BuildOption::operator=(const BuildOption& from)
{
	m_buildInfo = from.m_buildInfo;
	m_signalsStatePath = from.m_signalsStatePath;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

WindowsOption::WindowsOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

WindowsOption::WindowsOption(const WindowsOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

WindowsOption::~WindowsOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void WindowsOption::load()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_optionsWindowPos = s.value("OptionsDialog/pos", QPoint(200, 200)).toPoint();
	m_optionsWindowGeometry = s.value("OptionsDialog/geometry").toByteArray();
}

// -------------------------------------------------------------------------------------------------------------------

void WindowsOption::save()
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("OptionsDialog/pos", m_optionsWindowPos);
	s.setValue("OptionsDialog/geometry", m_optionsWindowGeometry);
}

// -------------------------------------------------------------------------------------------------------------------

WindowsOption& WindowsOption::operator=(const WindowsOption& from)
{
	m_mainWindowPos = from.m_mainWindowPos;
	m_mainWindowGeometry = from.m_mainWindowGeometry;
	m_mainWindowState = from.m_mainWindowState;

	m_optionsWindowPos = from.m_optionsWindowPos;
	m_optionsWindowGeometry = from.m_optionsWindowGeometry;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool compareDouble(double lDouble, double rDouble)
{
	return std::nextafter(lDouble, std::numeric_limits<double>::lowest()) <= rDouble && std::nextafter(lDouble, std::numeric_limits<double>::max()) >= rDouble;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
	m_windows.load();
	m_build.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
	m_windows.save();
	m_build.save();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::unload()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool Options::readFromXml()
{
	bool result = false;

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
	m_mutex.lock();

		m_build = from.m_build;
		m_windows = from.m_windows;

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
