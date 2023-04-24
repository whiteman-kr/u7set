#pragma once

#include "../CommonLib/HostAddressPort.h"
#include "../ClientLib//TuningUserManager.h"
#include "../TestSuiteLib/TestSuiteSettings.h"
#include "../TestSuiteLib/TestSuite.h"

//
// AppConfigSettings
//

class AppConfigSettings
{
public:
	AppConfigSettings();
	AppConfigSettings(const AppConfigSettings& That);

	void StoreUser();
	void RestoreUser();

	void StoreSystem();
	void RestoreSystem();

	TestSuite::TestSuiteSettings& librarySettings();
	const TestSuite::TestSuiteSettings& librarySettings() const;

	QStringList instanceHistory() const;
	void setInstanceHistory(const QStringList& value);

	QString language() const;
	void setLanguage(const QString& value);

	bool useLocalScriptsPath() const;
	void setUseLocalScriptsPath(bool value);

	QString localScriptsPath() const;
	void setLocalScriptsPath(const QString& path);

	QString localAppDataPath();

	//
	const QStringList& outputSearchCompleter() const;
	QStringList& outputSearchCompleter();

public:
	int m_requestInterval = 100;

	bool m_useLocalScriptsPath = false;
	QString m_localScriptsPath;

	// MainWindow options

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

private:
	QStringList m_outputSerachCompleter;

private:

	// System settings set by operator

	TestSuite::TestSuiteSettings m_librarySettings;

	QString m_language = "en";

	// User settings

	QStringList m_instanceHistory;
	QString m_localAppDataPath;

	mutable QMutex m;

public:
	AppConfigSettings& operator =(const AppConfigSettings& That)
	{
		m_librarySettings = That.m_librarySettings;

		m_useLocalScriptsPath = That.m_useLocalScriptsPath;
		m_localScriptsPath = That.m_localScriptsPath;
		m_language = That.m_language;

		return *this;
	};

};

extern AppConfigSettings theSettings;

