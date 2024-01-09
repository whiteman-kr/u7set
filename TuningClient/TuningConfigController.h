#pragma once

#include <map>
#include <set>
#include "../SchemaClientLib/SchemaClientConfigController.h"
#include "../ClientLib/TuningTcpClient.h"

//
// ConfigSettings
//
struct TuningClientConfigSettings
{
	int configurationId = -1;

	ClientLib::ConfigurationInfo configInfo;

	TuningClientSettings clientSettings;	//BuildInfo buildInfo;

	QString scriptGlobal;

	TuningClientSettings::LmStatusFlagMode lmStatusFlagMode() const
	{
		return static_cast<TuningClientSettings::LmStatusFlagMode>(clientSettings.statusFlagFunction);
	}
};


class TuningConfigController : public SchemaClientLib::SchemaClientConfigController
{
	Q_OBJECT

public:
	TuningConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);

	// Methods
	//
public:

protected:
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const BuildFileInfoArray& files) override;

	void dump(const TuningClientConfigSettings& conf) const;

	// Signals
	//
signals:
	void signalsArrived(QByteArray data);
	void filtersArrived(QByteArray data);

	void configurationArrived(TuningClientConfigSettings configuration);

	// Public properties
	//
public:
	TuningClientConfigSettings configuration() const;
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
	TuningClientConfigSettings m_configuration;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	inline static int s_configurationIdCounter = 0;
};

