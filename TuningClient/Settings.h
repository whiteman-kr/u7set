#ifndef SETTINGS_H
#define SETTINGS_H

#include "../lib/HostAddressPort.h"

class TuningPageSettings
{
public:
	int m_columnCount = 0;
	std::vector<int> m_columnsIndexes;
	std::vector<int> m_columnsWidth;
};

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

	bool filterByEquipment() const;
	void setFilterByEquipment(bool value);

	bool filterBySchema() const;
	void setFilterBySchema(bool value);

    QString language() const;
    void setLanguage(const QString& value);

	bool admin() const;

	TuningPageSettings *tuningPageSettings(int index);

    QString globalAppDataPath();

    QString localAppDataPath();

public:

    int m_requestInterval = 100;

	//

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	QByteArray m_mainWindowSplitterState;


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

	bool m_filterByEquipment = true;
	bool m_filterBySchema = true;

    QString m_language = "en";

    QString m_globalAppDataPath;
    QString m_localAppDataPath;

	QMutex m;

};

extern Settings theSettings;

#endif // SETTINGS_H
