#pragma once

//#include "../AppSignalLib/ComparatorSet.h"
#include "../ClientLib/ConfigController.h"
//#include "../lib/ClientBehavior.h"
#include "../VFrame30/Schema.h"


struct ConfigSettings
{
	int configurationId = -1;		// Counter to detect that configuration was updated

	ClientLib::ConfigurationInfo configInfo;

	std::vector<SoftwareEndpoint::DiagDataService> diagDataServices;
	//std::vector<SoftwareEndpoint::DiagDataService> diagDataRealTimeServices;
	//std::vector<SoftwareEndpoint::ArchiveService> archiveServices;

	QString startSchemaId;
	QString globalScript;
	QPixmap logoImage;
	//MonitorBehavior monitorBeahvior;

	// Tuning settings
	//
	//bool tuningEnabled = false;
	//std::vector<SoftwareEndpoint::TuningService> tuningServices;

	//bool tuningLogin = false;
	//QStringList tuningUserAccounts;
	//int tuningSessionTimeout = 0;
};


class DiagConfigController : public ClientLib::ConfigController
{
	Q_OBJECT

public:
	DiagConfigController() = delete;

	DiagConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);
	virtual ~DiagConfigController() = default;


protected:
	/// This function is called when the new configuarion arrives, it is overrided to get specific Diagnostics
	/// configuration, after it signal `configurationArrived` is emitted
	///
	virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const DiagnosticsSettings& settings, const BuildFileInfoArray& files) override;

	void dump(const ConfigSettings& conf) const;

	// Signals
	//
signals:
	/// These signals are emitted when the new configuration arrived and fully parsed.
	///
	void configurationUpdated();
	//void tuningSignalsArrived(QByteArray data);
	void configurationArrived(ConfigSettings configuration);

	// Public properties
	//
public:
	VFrame30::SchemaDetailsSet schemasDetailsSet() const;
	std::vector<VFrame30::SchemaDetails> schemasDetails() const;
	//std::set<QString> schemaAppSignals(const QString& schemaId);

	//QStringList schemasByAppSignalId(const QString& appSignalId) const;
	//QStringList schemasByLoopbackId(const QString& loopbackId) const;

	int configurationId() const;

	ConfigSettings configuration() const;
	ClientLib::ConfigurationInfo configInfo() const;

	QString configurationStartSchemaId() const;

	int schemaCount() const;
	QString schemaCaptionById(const QString& schemaId) const;
	QString schemaCaptionByIndex(int schemaIndex) const;
	QString schemaIdByIndex(int schemaIndex) const;

	//std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> trendSchemaItems() const;

	//const ComparatorSet& setpoints() const;

	// Data section
	//
private:
	mutable QReadWriteLock m_schemaDetailsLock;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	inline static int s_configurationIdCounter = 0;

	mutable QReadWriteLock m_confugurationLock;		// for access only to m_configuration
	ConfigSettings m_configuration;

	//ComparatorSet m_setpoints;
};


