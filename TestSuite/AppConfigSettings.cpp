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

}

void AppConfigSettings::RestoreUser()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	QMutexLocker l(&m);

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	m_language = s.value("MainWindow/language", m_language).toString();
}

TestLibrarySettings& AppConfigSettings::librarySettings()
{
	return m_librarySettings;
}

const TestLibrarySettings& AppConfigSettings::librarySettings() const
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

QString AppConfigSettings::localAppDataPath()
{
	return m_localAppDataPath;
}

AppConfigSettings theSettings;

