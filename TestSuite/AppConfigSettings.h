#pragma once

#include "../ClientLib/TuningUserManager.h"
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

public:
	struct Data
	{
		// Settings set by operator
		//
		TestSuite::TestSuiteSettings m_librarySettings;

		bool m_useLocalScriptsPath = false;
		QString m_localScriptsPath;

		QString m_language{"en"};
	};

public:
	static AppConfigSettings& instance();

	void save() const;
	void load();

	bool saveToFile(QString fileName) const;
	bool loadFromFile(QString fileName);

	bool wasLoadedFromFile() const;

private:
	void saveSystem(QSettings& s) const;
	void loadSystem(const QSettings& s);

public:
	QString localAppDataPath();

	const Data& data() const;
	Data& data();
	void setData(const Data& src);

	TestSuite::TestSuiteSettings& librarySettings();
	const TestSuite::TestSuiteSettings& librarySettings() const;

	QString language() const;

	bool useLocalScriptsPath() const;
	QString localScriptsPath() const;

private:

	// Local settings
	//
	QString m_localAppDataPath;
	Data m_data;
	bool m_wasLoadedFromFile = false;
};
