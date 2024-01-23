#ifndef SETTINGS_H
#define SETTINGS_H

#include "../CommonLib/HostAddressPort.h"
#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/SoftwareSettings.h"

extern QColor redColor;

//
// TuningClientAppSettings
//

class TuningClientAppSettings
{
public:
	TuningClientAppSettings();

public:
	struct UserData
	{
		int m_requestInterval{100};

		// MainWindow options

		QPoint m_mainWindowPos;
		QByteArray m_mainWindowGeometry;
		QByteArray m_mainWindowState; // Toolbars/dock's

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
	};

	struct SystemData
	{
		QString m_instanceStrId{"SYSTEMID_WS00_TUN"};

		QString m_configuratorIpAddress1{"127.0.0.1"};
		int m_configuratorPort1{PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST};

		QString m_configuratorIpAddress2{"127.0.0.1"};
		int m_configuratorPort2{PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST};

		QString m_language{"en"};

		QString m_filtersDefaultFile;

		bool m_useFiltersCustomFile{false};
		QString m_filtersCustomFile;
	};

	// Public methods
	//
public:
	static TuningClientAppSettings& instance();

	void save() const;
	void load();

	bool saveToFile(QString fileName) const;
	bool loadFromFile(QString fileName);

	bool wasLoadedFromFile() const;

	void saveUser();
	void loadUser();

private:
	void saveSystem(QSettings& s) const;
	void loadSystem(const QSettings& s);

public:
	QString localAppDataPath();

	const SystemData& system() const;
	SystemData& system();
	void setSystem(const SystemData& src);

	const UserData& user() const;
	UserData& user();
	void setUser(const UserData& src);

	//--
	//
	QString instanceStrId();

	HostAddressPort configuratorAddress1();
	HostAddressPort configuratorAddress2();

	QString language() const;

	QString userFiltersFile(); // Returns default of custom file depending on useCustom flag
	bool useCustomFiltersFile();
	QString customFiltersFile();

private:
	QString m_localAppDataPath;

	SystemData m_systemData;
	UserData m_userData;

	bool m_wasLoadedFromFile = false;
};

#endif // SETTINGS_H
