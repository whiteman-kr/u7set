#include "AppConfigSettings.h"
#include <QStandardPaths>
#include <QDir>

QColor redColor = QColor(192, 0, 0);

//
// Settings
//

AppConfigSettings::AppConfigSettings()
{
	// Determine the Local settings folder
	//
	m_localAppDataPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	QDir dir(m_localAppDataPath);
	if (dir.exists() == false)
	{
		dir.mkpath(m_localAppDataPath);
	}
}

AppConfigSettings::AppConfigSettings(const AppConfigSettings& That):
	AppConfigSettings()
{
	*this = That;
}

AppConfigSettings& AppConfigSettings::instance()
{
	static AppConfigSettings theSettings;
	return theSettings;
}


void AppConfigSettings::save() const
{
	QSettings s{qApp->organizationName(), qApp->applicationName()};	// Explicitly point app name, as it can be changed via settings.
	saveSystem(s);
	return;
}

void AppConfigSettings::load()
{
	QSettings s{qApp->organizationName(), qApp->applicationName()};	// Explicitly point app name, as it can be changed via settings.
	loadSystem(s);
	m_wasLoadedFromFile = false;
	return;
}

bool AppConfigSettings::saveToFile(QString fileName) const
{
	QSettings s{fileName, QSettings::IniFormat};
	saveSystem(s);
	s.sync();
	return s.status() == QSettings::Status::NoError;
}

bool AppConfigSettings::loadFromFile(QString fileName)
{
	QSettings s{fileName, QSettings::IniFormat};
	loadSystem(s);
	m_wasLoadedFromFile = true;
	return s.status() == QSettings::Status::NoError;
}

bool AppConfigSettings::wasLoadedFromFile() const
{
	return m_wasLoadedFromFile;
}

void AppConfigSettings::saveSystem(QSettings& s) const
{
	// save system settings
	//
	m_data.m_librarySettings.saveToRegistry(s);

	s.setValue("AppConfigSettings/language", m_data.m_language);

	s.setValue("AppConfigSettings/useLocalScriptsPath", m_data.m_useLocalScriptsPath);
	s.setValue("AppConfigSettings/localScriptsPath", m_data.m_localScriptsPath);
}

void AppConfigSettings::loadSystem(const QSettings& s)
{
	// read system settings
	//
	m_data.m_librarySettings.restoreFromRegistry(s);

	m_data.m_language = s.value("AppConfigSettings/language", m_data.m_language).toString();

	m_data.m_useLocalScriptsPath = s.value("AppConfigSettings/useLocalScriptsPath", m_data.m_useLocalScriptsPath).toBool();
	m_data.m_localScriptsPath = s.value("AppConfigSettings/localScriptsPath", m_data.m_localScriptsPath).toString();
}

QString AppConfigSettings::localAppDataPath()
{
	return m_localAppDataPath;
}

const AppConfigSettings::Data& AppConfigSettings::data() const
{
	return m_data;
}

AppConfigSettings::Data& AppConfigSettings::data()
{
	return m_data;
}

void AppConfigSettings::setData(const AppConfigSettings::Data& src)
{
	m_data = src;
}

TestSuite::TestSuiteSettings& AppConfigSettings::librarySettings()
{
	return m_data.m_librarySettings;
}

const TestSuite::TestSuiteSettings& AppConfigSettings::librarySettings() const
{
	return m_data.m_librarySettings;
}

QString AppConfigSettings::language() const
{
	return m_data.m_language;
}

bool AppConfigSettings::useLocalScriptsPath() const
{
	return m_data.m_useLocalScriptsPath;
}

QString AppConfigSettings::localScriptsPath() const
{
	return m_data.m_localScriptsPath;
}

