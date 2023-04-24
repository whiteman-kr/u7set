#include "AppConfigSettings.h"

QColor redColor = QColor(192, 0, 0);

//
// Settings
//

AppConfigSettings::AppConfigSettings():
	m_language("en")
{
}

AppConfigSettings::AppConfigSettings(const AppConfigSettings& That):
	AppConfigSettings()
{
	*this = That;
}

void AppConfigSettings::StoreSystem()
{
	// save system settings
	//
	m_librarySettings.saveToRegistry();

	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QString instanceHistoryString = m_instanceHistory.join(';');
	s.setValue("m_instanceHistory", instanceHistoryString);
}

void AppConfigSettings::RestoreSystem()
{
	// read system settings
	//
	m_librarySettings.restoreFromRegistry();

	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QString instanceHistoryString = s.value("m_instanceHistory", QString()).toString();
	m_instanceHistory = instanceHistoryString.split(';', Qt::SkipEmptyParts);

	// Determine the Local settings folder

	m_localAppDataPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	QDir dir(m_localAppDataPath);

	if (dir.exists() == false)
	{
		dir.mkpath(m_localAppDataPath);
	}
}


void AppConfigSettings::StoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("MainWindow/language", m_language);

	s.setValue("Global/useLocalScriptsPath", m_useLocalScriptsPath);
	s.setValue("Global/localScriptsPath", m_localScriptsPath);

	s.setValue("TestLogTabPage/m_buildSerachCompleter", m_outputSerachCompleter);
}

void AppConfigSettings::RestoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_language = s.value("MainWindow/language", m_language).toString();

	m_useLocalScriptsPath = s.value("Global/useLocalScriptsPath", m_useLocalScriptsPath).toBool();
	m_localScriptsPath = s.value("Global/localScriptsPath", m_localScriptsPath).toString();
	m_outputSerachCompleter = s.value("TestLogTabPage/m_buildSerachCompleter").toStringList();

}

TestSuite::TestSuiteSettings& AppConfigSettings::librarySettings()
{
	return m_librarySettings;
}

const TestSuite::TestSuiteSettings& AppConfigSettings::librarySettings() const
{
	return m_librarySettings;
}

QStringList AppConfigSettings::instanceHistory() const
{
	QMutexLocker l(&m);
	return m_instanceHistory;
}

void AppConfigSettings::setInstanceHistory(const QStringList& value)
{
	QMutexLocker l(&m);
	m_instanceHistory = value;
}


QString AppConfigSettings::language() const
{
	return m_language;
}

void AppConfigSettings::setLanguage(const QString& value)
{
	m_language = value;
}

bool AppConfigSettings::useLocalScriptsPath() const
{
	return m_useLocalScriptsPath;
}

void AppConfigSettings::setUseLocalScriptsPath(bool value)
{
	m_useLocalScriptsPath = value;
}

QString AppConfigSettings::localScriptsPath() const
{
	return m_localScriptsPath;
}

void AppConfigSettings::setLocalScriptsPath(const QString& path)
{
	m_localScriptsPath = path;
}

QString AppConfigSettings::localAppDataPath()
{
	return m_localAppDataPath;
}

const QStringList& AppConfigSettings::outputSearchCompleter() const
{
	return m_outputSerachCompleter;
}

QStringList& AppConfigSettings::outputSearchCompleter()
{
	return m_outputSerachCompleter;
}


AppConfigSettings theSettings;

