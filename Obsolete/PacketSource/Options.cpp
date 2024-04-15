#include "Options.h"

#include <QSettings>

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
	QMutexLocker l(&m_mutex);

	m_build = from.m_build;
	m_windows = from.m_windows;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
