#ifndef SETTINGS_H
#define SETTINGS_H

#include "../CommonLib/HostAddressPort.h"
#include "UserManager.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/SoftwareSettings.h"

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

struct SchemaInfo
{
	SchemaInfo(const QString& id, const QString& caption, const std::set<QString>& tags)
	{
		m_id = id;
		m_caption = caption;
		m_tags = tags;
	}

	bool hasAnyTag(const QStringList& tags) const
	{
		for (const QString& tag : tags)
		{
			if (m_tags.find(tag.trimmed().toLower()) != m_tags.end())
			{
				return true;
			}
		}

		return false;
	}

	QString m_id;
	QString m_caption;
	std::set<QString> m_tags;
};

//
// ConfigSettings
//

struct ConfigSettings
{
	TuningClientSettings clientSettings;

	ConfigConnection serviceAddress;				// Tuning Service connection params

	std::vector<SchemaInfo> schemas;

	BuildInfo buildInfo;

	QString scriptGlobal;

	QString scriptConfigArrived;

	QString errorMessage;				// Parsing error message, empty if no errors

	// Warning! Add new values to copy operator!!!

	LmStatusFlagMode lmStatusFlagMode() const
	{
		return static_cast<LmStatusFlagMode>(clientSettings.statusFlagFunction);
	}

	ConfigSettings& operator = (const ConfigSettings& That)
	{
		serviceAddress = That.serviceAddress;
		clientSettings = That.clientSettings;

		schemas = That.schemas;
		buildInfo = That.buildInfo;

		scriptGlobal = That.scriptGlobal;
		scriptConfigArrived = That.scriptConfigArrived;

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

	QString userFiltersFile();	// Returns default of custom file depending on useCustom flag

	bool useCustomFiltersFile();
	void setUseCustomFiltersFile(bool value);

	QString customFiltersFile();
	void setCustomFiltersFile(const QString& value);

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

	QStringList m_tuningWorkspaceMasks;

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

	QString m_filtersDefaultFile;

	bool m_useFiltersCustomFile = false;
	QString m_filtersCustomFile;

	QMutex m;

};

extern Settings theSettings;

extern ConfigSettings theConfigSettings;

#endif // SETTINGS_H
