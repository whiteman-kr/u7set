#pragma once

#include <map>
#include <set>
#include "../VFrame30/Schema.h"
#include "../ClientLib/ConfigController.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../OnlineLib/MatsUsers.h"

//
// ConfigSettings
//
struct ConfigSettings
{
	int configurationId = -1;

	ClientLib::ConfigurationInfo configInfo;

	TuningClientSettings clientSettings;	//BuildInfo buildInfo;

	QString scriptGlobal;

	OnlineLib::MatsUserStorage matsUsers;

	TuningClientSettings::LmStatusFlagMode lmStatusFlagMode() const
	{
		return static_cast<TuningClientSettings::LmStatusFlagMode>(clientSettings.statusFlagFunction);
	}
};


class TuningConfigController : public ClientLib::ConfigController
{
	Q_OBJECT

public:
	TuningConfigController() = delete;

	TuningConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);
	virtual ~TuningConfigController() = default;

	// Methods
	//
public:
	int schemaCount() const;
	QString schemaCaptionById(const QString& schemaId) const;
	QString schemaCaptionByIndex(int schemaIndex) const;
	QString schemaIdByIndex(int schemaIndex) const;
	std::set<QString> schemaTagsByIndex(int schemaIndex) const;
	bool schemaHasTags(int schemaIndex, const QStringList& tags) const;

protected:
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const BuildFileInfoArray& files) override;

	void dump(const ConfigSettings& conf) const;

	// Signals
	//
signals:
	void signalsArrived(QByteArray data);
	void filtersArrived(QByteArray data);

	void configurationArrived(ConfigSettings configuration);

	// Public properties
	//
public:
	ConfigSettings configuration() const;
	ClientLib::ConfigurationInfo configInfo() const;

	QString startSchemaId() const;
	bool showSignals() const;
	bool showSchemas() const;

	TuningClientSettings::LmStatusFlagMode lmStatusFlagMode() const;
	bool singleLmControlMode() const;

	// Data section
	//
private:
	std::map<QString, QString> m_filesMD5Map;	// Key is full file path, value is file MD5

	mutable QReadWriteLock m_lock;
	ConfigSettings m_configuration;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	inline static int s_configurationIdCounter = 0;
};

