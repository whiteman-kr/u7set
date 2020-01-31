#include "Options.h"

#include <QSettings>
#include <QFile>
#include <QCryptographicHash>

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BuildFile::BuildFile()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

BuildFile::~BuildFile()
{
}

// -------------------------------------------------------------------------------------------------------------------

void BuildFile::clear()
{
	m_path.clear();
	m_fileName.clear();
	m_size = 0;
	m_md5.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildFile::setPath(const QString& path)
{
	m_path = path;

	if (m_path.isEmpty() == true)
	{
		return;
	}

	QStringList list = m_path.split(BUILD_FILE_SEPARATOR);
	if (list.count() >= 2)
	{
		m_fileName = BUILD_FILE_SEPARATOR + list[ list.count() - 2 ] + BUILD_FILE_SEPARATOR + list[ list.count() - 1 ];
	}

	QFile file(m_path);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		return;
	}

	m_size = file.size();

	QCryptographicHash md5Generator(QCryptographicHash::Md5);
	md5Generator.addData(&file);
	m_md5 = md5Generator.result().toHex();

	file.close();
}

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
	for (int i = 0; i < BUILD_FILE_TYPE_COUNT; i ++)
	{
		m_buildFile[i].clear();
	}

	m_enableReload = true;
	m_timeoutReload = BUILD_FILE_RELOAD_TIMEOUT;

	m_appDataSrvIP.clear();
	m_ualTesterIP.clear();

	m_signalsStatePath.clear();

}

// -------------------------------------------------------------------------------------------------------------------

BuildFile BuildOption::buildFile(int type) const
{
	if (type < 0 || type >= BUILD_FILE_TYPE_COUNT)
	{
		return BuildFile();
	}

	return m_buildFile[type];
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::setBuildFile(int type, const BuildFile& buildFile)
{
	if (type < 0 || type >= BUILD_FILE_TYPE_COUNT)
	{
		return;
	}

	m_buildFile[type] = buildFile;
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::load()
{
	QSettings s;

	m_buildDirPath = s.value(QString("%1BuildDirPath").arg(BUILD_REG_KEY), QString()).toString();

	for (int i = 0; i < BUILD_FILE_TYPE_COUNT; i ++)
	{
		QString path = s.value(QString("%1%2").arg(BUILD_REG_KEY).arg(BuildFileRegKey[i]), QString()).toString();
		m_buildFile[i].setPath(path);
	}

	m_enableReload = s.value(QString("%1EnableReloadBuildFiles").arg(BUILD_REG_KEY), true).toBool();
	m_timeoutReload = s.value(QString("%1TimeoutReloadBuildFiles").arg(BUILD_REG_KEY), BUILD_FILE_RELOAD_TIMEOUT).toInt();

	m_appDataSrvIP = s.value(QString("%1AppDataSrvIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString();
	m_ualTesterIP = s.value(QString("%1UalTesterIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString();

	m_signalsStatePath = s.value(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), QString("SignalStates.csv")).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::save()
{
	QSettings s;

	s.setValue(QString("%1BuildDirPath").arg(BUILD_REG_KEY), m_buildDirPath);

	for (int i = 0; i < BUILD_FILE_TYPE_COUNT; i ++)
	{
		s.setValue(QString("%1%2").arg(BUILD_REG_KEY).arg(BuildFileRegKey[i]), m_buildFile[i].path());
	}

	s.setValue(QString("%1AppDataSrvIP").arg(BUILD_REG_KEY), m_appDataSrvIP);
	s.setValue(QString("%1UalTesterIP").arg(BUILD_REG_KEY), m_ualTesterIP);

	s.setValue(QString("%1EnableReloadBuildFiles").arg(BUILD_REG_KEY), m_enableReload);
	s.setValue(QString("%1TimeoutReloadBuildFiles").arg(BUILD_REG_KEY), m_timeoutReload);

	s.setValue(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), m_signalsStatePath);
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption& BuildOption::operator=(const BuildOption& from)
{
	m_buildDirPath = from.m_buildDirPath;
	for (int i = 0; i < BUILD_FILE_TYPE_COUNT; i ++)
	{
		m_buildFile[i] = from.m_buildFile[i];
	}

	m_enableReload = from.m_enableReload;
	m_timeoutReload = from.m_timeoutReload;

	m_appDataSrvIP = from.m_appDataSrvIP;
	m_ualTesterIP = from.m_ualTesterIP;

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

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
