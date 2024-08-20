#pragma once

#include <map>
#include <set>
#include <SchemaClientLib/SchemaClientConfigController.h>
#include "../OnlineLib/MatsUsers.h"

//
// ConfigSettings
//
struct TuningClientConfigSettings
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


class TuningConfigController : public SchemaClientLib::SchemaClientConfigController
{
	Q_OBJECT

public:
	TuningConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);

	// Methods
	//
public:

protected:
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files) override;

	void dump(const TuningClientConfigSettings& conf) const;

	// Signals
	//
signals:
	void signalsArrived(QByteArray data);
	void configurationArrived(TuningClientConfigSettings configuration);

	// Public properties
	//
public:
	TuningClientConfigSettings configuration() const;
	ClientLib::ConfigurationInfo configInfo() const;

	const QByteArray& tuningUiData() const;

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

	QByteArray m_tuningUiData;

	inline static int s_configurationIdCounter = 0;
};

