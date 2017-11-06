#ifndef SETTINGS_H
#define SETTINGS_H

#include "../lib/HostAddressPort.h"

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
	ConfigConnection tuns1;				// Tuning Service connection params
	ConfigConnection tuns2;				// Tuning Service connection params

	bool autoApply = true;

	bool showSignals = true;

	bool showSchemas = true;

	bool showSchemasList = true;

	bool filterByEquipment = true;

	bool filterBySchema = true;

	QStringList equipmentList;

	std::vector<SchemaSettings> schemas;

	QString errorMessage;				// Parsing error message, empty if no errors
};

//
// TuningPageSettings
//


class TuningPageSettings
{
public:
	int m_columnCount = 0;
	std::vector<int> m_columnsIndexes;
	std::vector<int> m_columnsWidth;
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

	QString instanceStrId();
	void setInstanceId(const QString& value);

	void setConfiguratorAddress1(const QString& address, int port);
	HostAddressPort configuratorAddress1();

	void setConfiguratorAddress2(const QString& address, int port);
	HostAddressPort configuratorAddress2();

	QString language() const;
	void setLanguage(const QString& value);

	bool admin() const;

	TuningPageSettings* tuningPageSettings(int index);

	QString globalAppDataPath();

	QString localAppDataPath();

	QString userFiltersFile();

public:

	int m_requestInterval = 10;

	//

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	QByteArray m_tuningWorkspaceSplitterState;
	QByteArray m_schemasWorkspaceSplitterState;


	// Property Editor Options
	//
	QPoint m_multiLinePropertyEditorWindowPos;
	QByteArray m_multiLinePropertyEditorGeometry;

	int m_presetPropertiesSplitterState;
	QPoint m_presetPropertiesWindowPos;
	QByteArray m_presetPropertiesWindowGeometry;

	// Preset Editor options

	QPoint m_presetEditorPos;
	QByteArray m_presetEditorGeometry;


	QByteArray m_tuningFiltersDialogChooseSignalGeometry;
	int m_tuningFiltersPropertyEditorSplitterPos = -1;

private:

	// Tuning pages settings
	//
	std::vector<TuningPageSettings> m_tuningPageSettings;

	bool m_admin = false;

	QString m_instanceStrId;

	QString m_configuratorIpAddress1;
	int m_configuratorPort1;

	QString m_configuratorIpAddress2;
	int m_configuratorPort2;

	QString m_language = "en";

	QString m_globalAppDataPath;
	QString m_localAppDataPath;

	QString m_userFiltersFile;

	QMutex m;

};

extern Settings theSettings;

extern ConfigSettings theConfigSettings;

#endif // SETTINGS_H
