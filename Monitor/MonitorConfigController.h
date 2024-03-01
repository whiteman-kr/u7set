#pragma once

#include "../AppSignalLib/ComparatorSet.h"
#include "../SchemaClientLib/SchemaClientConfigController.h"
#include "../OnlineLib/MatsUsers.h"

#include <Behavior/MonitorBehavior.h>


struct MonitorConfigSettings
{
	int configurationId = -1;		// Counter to detect that configuration was updated

	ClientLib::ConfigurationInfo configInfo;

	std::vector<SoftwareEndpoint::AppDataService> appDataServices;
	std::vector<SoftwareEndpoint::AppDataService> appDataRealTimeServices;
	std::vector<SoftwareEndpoint::ArchiveService> archiveServices;

	QString startSchemaId;
	QString globalScript;
	QPixmap logoImage;
	Behavior::MonitorBehavior monitorBehavior;

	// Tuning settings
	//
	bool tuningEnabled = false;
	std::vector<SoftwareEndpoint::TuningService> tuningServices;
	OnlineLib::MatsUserStorage matsUsers;

	bool tuningLogin = false;
	QStringList tuningUserAccounts;
	int tuningSessionTimeout = 0;
};


class MonitorConfigController : public SchemaClientLib::SchemaClientConfigController
{
	Q_OBJECT

public:
	MonitorConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);

protected:
	/// This function is called when the new configuration arrives, it is overriden to get specific Monitor
	/// configuration, after it signal `configurationArrived` is emitted
	///
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const MonitorSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files) override;

	void dump(const MonitorConfigSettings& conf) const;

	// Signals
	//
signals:
	// These signals are emitted when the new configuration arrived and fully parsed.
	//
	void configurationUpdated();
	void tuningSignalsArrived(QByteArray data);
	void configurationArrived(MonitorConfigSettings configuration);

	// Public properties
	//
public:
	int configurationId() const;

	MonitorConfigSettings configuration() const;
	ClientLib::ConfigurationInfo configInfo() const;

	QString configurationStartSchemaId() const;

	const ComparatorSet& setpoints() const;

	// Data section
	//
private:
	inline static int s_configurationIdCounter = 0;

	mutable QReadWriteLock m_configurationLock;		// for access only to m_configuration
	MonitorConfigSettings m_configuration;

	ComparatorSet m_setpoints;
};


