#pragma once

#include <map>
#include <set>
#include "../VFrame30/Schema.h"
#include "../ClientLib/ConfigController.h"
#include "../lib/Tuning/TuningTcpClient.h"

//
// ConfigSettings
//
struct ConfigSettings
{
	int configurationId = -1;

	Client::ConfigurationInfo configInfo;

	TuningClientSettings clientSettings;	//BuildInfo buildInfo;

	QString scriptGlobal;
	QString scriptConfigArrived;

	LmStatusFlagMode lmStatusFlagMode() const
	{
		return static_cast<LmStatusFlagMode>(clientSettings.statusFlagFunction);
	}
};


class TuningConfigController : public Client::ConfigController
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

protected:
	virtual bool updateConfiguration(const Client::ConfigurationInfo& conf, const TuningClientSettings& settings, const BuildFileInfoArray& files) override;

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
	Client::ConfigurationInfo configInfo() const;

	QString startSchemaId() const;
	bool showSignals() const;
	bool showSchemas() const;
	LmStatusFlagMode lmStatusFlagMode() const;

	// Data section
	//
private:
	std::map<QString, QString> m_filesMD5Map;	// Key is full file path, value is file MD5

	mutable QReadWriteLock m_lock;
	ConfigSettings m_configuration;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	inline static int s_configurationIdCounter = 0;
};

