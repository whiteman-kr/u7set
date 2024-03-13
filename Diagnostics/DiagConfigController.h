#pragma once

#include <SchemaClientLib/SchemaClientConfigController.h>
#include "../VFrame30/Schema.h"


struct DiagConfigSettings
{
	int configurationId = -1;		// Counter to detect that configuration was updated

	ClientLib::ConfigurationInfo configInfo;

	std::vector<SoftwareEndpoint::DiagDataService> diagDataServices;
	std::vector<SoftwareEndpoint::DiagDataService> diagDataRealTimeServices;
	std::vector<SoftwareEndpoint::ArchiveService> archiveServices;

	QString startSchemaId;
	QString globalScript;
	QPixmap logoImage;
	//MonitorBehavior monitorBehavior;
};


class DiagConfigController : public SchemaClientLib::SchemaClientConfigController
{
	Q_OBJECT

public:
	DiagConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);

protected:
	/// This function is called when the new configuration arrives, it is overriden to get specific Diagnostics
	/// configuration, after it signal `configurationArrived` is emitted
	///
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const DiagnosticsSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files) override;

	void dump(const DiagConfigSettings& conf) const;

	// Signals
	//
signals:
	/// These signals are emitted when the new configuration arrived and fully parsed.
	///
	void configurationUpdated();
	void configurationArrived(DiagConfigSettings configuration);

	// Public properties
	//
public:
	int configurationId() const;

	DiagConfigSettings configuration() const;
	ClientLib::ConfigurationInfo configInfo() const;

	QString configurationStartSchemaId() const;

	// Data section
	//
private:
	inline static int s_configurationIdCounter = 0;

	mutable QReadWriteLock m_configurationLock;		// for access only to m_configuration
	DiagConfigSettings m_configuration;
};


