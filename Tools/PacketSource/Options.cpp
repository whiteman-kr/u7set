#include "Options.h"

#include <QSettings>

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

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
	m_buildDirPath.clear();
	m_signalsFilePath.clear();
	m_sourceCfgFilePath.clear();
	m_sourcesFilePath.clear();

	m_appDataSrvIP.clear();
	m_ualTesterIP.clear();
}


// -------------------------------------------------------------------------------------------------------------------

void BuildOption::load()
{
	QSettings s;

	m_buildDirPath = s.value(QString("%1BuildDirPath").arg(SOURCE_REG_KEY), QString()).toString();
	m_signalsFilePath = s.value(QString("%1SignalsFilePath").arg(SOURCE_REG_KEY), QString()).toString();
	m_sourceCfgFilePath = s.value(QString("%1SourceCfgFilePath").arg(SOURCE_REG_KEY), QString()).toString();
	m_sourcesFilePath = s.value(QString("%1SourcesFilePath").arg(SOURCE_REG_KEY), QString()).toString();

	m_appDataSrvIP = s.value(QString("%1AppDataSrvIP").arg(SOURCE_REG_KEY), QString("127.0.0.1")).toString();
	m_ualTesterIP = s.value(QString("%1UalTesterIP").arg(SOURCE_REG_KEY), QString("127.0.0.1")).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::save()
{
	QSettings s;

	s.setValue(QString("%1BuildDirPath").arg(SOURCE_REG_KEY), m_buildDirPath);
	s.setValue(QString("%1SignalsFilePath").arg(SOURCE_REG_KEY), m_signalsFilePath);
	s.setValue(QString("%1SourceCfgFilePath").arg(SOURCE_REG_KEY), m_sourceCfgFilePath);
	s.setValue(QString("%1SourcesFilePath").arg(SOURCE_REG_KEY), m_sourcesFilePath);

	s.setValue(QString("%1AppDataSrvIP").arg(SOURCE_REG_KEY), m_appDataSrvIP);
	s.setValue(QString("%1UalTesterIP").arg(SOURCE_REG_KEY), m_ualTesterIP);
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption& BuildOption::operator=(const BuildOption& from)
{
	m_buildDirPath = from.m_buildDirPath;
	m_signalsFilePath = from.m_signalsFilePath;
	m_sourceCfgFilePath = from.m_sourceCfgFilePath;
	m_sourcesFilePath = from.m_sourcesFilePath;

	m_appDataSrvIP = from.m_appDataSrvIP;
	m_ualTesterIP = from.m_ualTesterIP;

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

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
