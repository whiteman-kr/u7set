#ifndef SETTINGS_H
#define SETTINGS_H

#include "../lib/HostAddressPort.h"
#include "UserManager.h"

// Enable the next line to access the admin functions
//#define USE_ADMIN_REGISTRY_AREA

extern QColor redColor;

//
// ConfigConnection
//

class ConfigConnection
{
	ConfigConnection() {}

public:
	ConfigConnection(QString EquipmentId, QString ipAddress, int port);

	QString equipmentId() const;
	QString ip() const;
	int port() const;

	HostAddressPort address() const;

protected:
	QString m_equipmentId;
	QString m_ip;
	int m_port;

	friend struct ConfigSettings;
};

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
// SchemaSettings
//

struct SchemaSettings
{
	SchemaSettings(const QString& id, const QString& caption)
	{
		m_id = id;
		m_caption = caption;
	}

	QString m_id;
	QString m_caption;
};

//
// ConfigSettings
//

struct ConfigSettings
{
	ConfigConnection tuningServiceAddress;				// Tuning Service connection params

	bool autoApply = true;

	LogonMode logonMode = LogonMode::Permanent;

	bool showSignals = true;

	bool showSchemas = true;

	bool showSchemasList = true;

	bool showSchemasTabs = true;

	bool filterByEquipment = true;

	bool filterBySchema = true;

	bool showSOR = true;

	bool useAccessFlag = false;

	int loginSessionLength = 120;

	QStringList usersAccounts;

	std::vector<SchemaSettings> schemas;

	BuildInfo buildInfo;

	QString errorMessage;				// Parsing error message, empty if no errors

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

	QString userFiltersFile();

public:

	int m_requestInterval = 100;

	bool m_enableSimulation = false;
	bool m_simulationMode = false;

	// MainWindow options

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	QByteArray m_tuningWorkspaceSplitterState;
	QByteArray m_schemasWorkspaceSplitterState;

	// DialogProperties Options

	int m_presetPropertiesSplitterState = 0;
	QPoint m_presetPropertiesWindowPos;
	QByteArray m_presetPropertiesWindowGeometry;

	// DialogFiltersEditor options

	QPoint m_dialogFiltersEditorPos = QPoint(-1, -1);
	QByteArray m_dialogFiltersEditorGeometry;

	QByteArray m_dialogFiltersEditorSplitterPosition;
	int m_dialogFiltersEditorPropertyEditorSplitterPosition = -1;

	// SwitchPresetsPage options

	int m_switchPresetsPageColCount = 8;
	int m_switchPresetsPageRowCount = 3;
	int m_switchPresetsPageButtonsWidth = 150;
	int m_switchPresetsPageButtonsHeight = 100;
	QByteArray m_switchPresetsPageSplitterPosition;

	// Columns Color settings

	QColor m_columnErrorBackColor = redColor;
	QColor m_columnErrorTextColor = Qt::white;

	QColor m_columnDisabledBackColor = Qt::white;
	QColor m_columnDisabledTextColor = Qt::darkGray;

	QColor m_columnUnappliedBackColor = Qt::gray;
	QColor m_columnUnappliedTextColor = Qt::white;

	QColor m_columnDefaultMismatchBackColor = Qt::yellow;
	QColor m_columnDefaultMismatchTextColor = Qt::black;

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

	QString m_userFiltersFile;


	QMutex m;

};

extern Settings theSettings;

extern ConfigSettings theConfigSettings;

#endif // SETTINGS_H
