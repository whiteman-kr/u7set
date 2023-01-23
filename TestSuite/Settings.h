#ifndef SETTINGS_H
#define SETTINGS_H

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
struct BuildInfo
{
	QString projectName;
	int buildNo = -1;
	QString configuration;
	QString date;
	int changeset = -1;
	QString user;
	QString workstation;

};

//
// ConfigSettings
//

struct ConfigSettings
{
	//TuningClientSettings clientSettings;

	QString errorMessage;				// Parsing error message, empty if no errors

	ConfigSettings& operator = (const ConfigSettings& That)
	{
		//clientSettings = That.clientSettings;

		return *this;
	}

};

//
// Settings
//

class Settings
{
public:
	Settings();

	void StoreUser();
	void RestoreUser();

	void StoreSystem();
	void RestoreSystem();

	QStringList instanceHistory();
	void setInstanceHistory(const QStringList& value);

	QString instanceStrId();
	void setInstanceStrId(const QString& value);

	void setConfiguratorAddress1(const QString& address, int port);
	HostAddressPort configuratorAddress1();

	void setConfiguratorAddress2(const QString& address, int port);
	HostAddressPort configuratorAddress2();

	QString language() const;
	void setLanguage(const QString& value);

#ifdef USE_ADMIN_REGISTRY_AREA
	bool admin() const;
#endif

	QString localAppDataPath();

public:

	int m_requestInterval = 100;

	// MainWindow options

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

private:

#ifdef USE_ADMIN_REGISTRY_AREA
	bool m_admin = false;
#endif

	QStringList m_instanceHistory;
	QString m_instanceStrId;

	QString m_configuratorIpAddress1;
	int m_configuratorPort1;

	QString m_configuratorIpAddress2;
	int m_configuratorPort2;

	QString m_language = "en";

	QString m_localAppDataPath;

	QMutex m;

};

extern Settings theSettings;

extern ConfigSettings theConfigSettings;

#endif // SETTINGS_H
