#pragma once

#include "../CommonLib/HostAddressPort.h"
#include "../lib/Tuning/TuningUserManager.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/SoftwareSettings.h"

// Enable the next line to access the admin functions
//#define USE_ADMIN_REGISTRY_AREA

//extern QColor redColor;

//
// ConfigConnection
//

//
// BuildInfo
//
/*struct BuildInfo
{
	QString projectName;
	int buildNo = -1;
	QString configuration;
	QString date;
	int changeset = -1;
	QString user;
	QString workstation;

};*/

//
// AppConfigSettings
//

/*struct AppConfigSettings
{
	//TuningClientSettings clientSettings;

	QString errorMessage;				// Parsing error message, empty if no errors

	AppConfigSettings& operator = (const AppConfigSettings& That)
	{
		//clientSettings = That.clientSettings;

		return *this;
	}

};*/

//
// Settings
//

class Settings
{
public:
	Settings();
	Settings(const Settings& That);

	void StoreUser();
	void RestoreUser();

	void StoreSystem();
	void RestoreSystem();

	QStringList instanceHistory() const;
	void setInstanceHistory(const QStringList& value);

	QString instanceStrId() const;
	void setInstanceStrId(const QString& value);

	void setConfiguratorAddress1(const QString& address, int port);
	HostAddressPort configuratorAddress1() const;

	void setConfiguratorAddress2(const QString& address, int port);
	HostAddressPort configuratorAddress2() const;

	QString language() const;
	void setLanguage(const QString& value);

	QString localAppDataPath();

	bool loadScriptsFromPath();
	void setLoadScriptsFromPath(bool value);

	QString loadScriptsPath();
	void setLoadScriptsPath(const QString& path);

public:
	int m_requestInterval = 100;

	// MainWindow options

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

private:

	// System settings set by operator

	QString m_instanceStrId;

	QString m_configuratorIpAddress1;
	int m_configuratorPort1;

	QString m_configuratorIpAddress2;
	int m_configuratorPort2;

	QString m_language = "en";

	bool m_loadScriptsFromPath = false;
	QString m_loadScriptsPath;

	// User settings

	QStringList m_instanceHistory;
	QString m_localAppDataPath;

	mutable QMutex m;

public:
	Settings& operator =(const Settings& That)
	{
		m_instanceStrId = That.m_instanceStrId;

		m_configuratorIpAddress1 = That.m_configuratorIpAddress1;
		m_configuratorPort1 = That.m_configuratorPort1;

		m_configuratorIpAddress2 = That.m_configuratorIpAddress2;
		m_configuratorPort2 = That.m_configuratorPort2;

		m_language = That.m_language;

		m_loadScriptsFromPath = That.m_loadScriptsFromPath;
		m_loadScriptsPath = That.m_loadScriptsPath;

		return *this;
	};

};

extern Settings theSettings;

