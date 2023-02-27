#pragma once

#include "../OnlineLib/CfgServerLoader.h"
#include "../UtilsLib/ILogFile.h"
#include "../lib/ClientBehavior.h"
#include "../VFrame30/Schema.h"


struct ConfigSettings
{
	int configurationId = -1;
	int buildNo = -1;
	QString softwareEquipmentId;
	QString project;
	QString startSchemaId;

	std::vector<MonitorSettings::AppDataService> appDataServices;
	std::vector<MonitorSettings::AppDataService> appDataRealTimeServices;

	std::vector<MonitorSettings::ArchiveService> archiveServices;

	// Tuning settings
	//
	bool tuningEnabled = false;
	std::vector<MonitorSettings::TuningService> tuningServices;

	bool tuningLogin = false;
	QStringList tuningUserAccounts;
	int tuningSessionTimeout = 0;

	//
	QString globalScript;
	QString onConfigurationArrivedScript;

	QImage logoImage;

	MonitorBehavior monitorBeahvior;

	QString errorMessage;				// Parsing error message, empty if no errors
};


class MonitorConfigController : public QObject, public HasLogFile
{
	Q_OBJECT

public:
	MonitorConfigController() = delete;

	MonitorConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);
	virtual ~MonitorConfigController();

	// Methods
	//
public:
	void setConnectionParams(QString equipmentId, HostAddressPort address1, HostAddressPort address2);

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFile(const QString& pathFileName, QByteArray* fileData);

	bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);
	bool getFileById(const QString& id, QByteArray* fileData);

	bool hasFileId(QString fileId) const;

	Tcp::ConnectionState getConnectionState() const;

	const SoftwareInfo& softwareInfo() const;

	// signals
	//
signals:
	void configurationUpdate();
	void configurationArrived(ConfigSettings configuration);
	void unknownClient(QString errMsg);										// Error if CfgService cannot find SoftwareID
	void wrongClientHostname(QString errMsg);

	// slots
	//
public slots:
	void start();

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData,
								 const BuildFileInfoArray buildFileInfoArray,
								 SessionParams sessionParams,
								 std::shared_ptr<const SoftwareSettings> curSettingsProfile);

private:
	bool xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigSettings* outSetting);
	bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting);

	bool applyCurSettingsProfile(std::shared_ptr<const SoftwareSettings> curSettingsProfile, ConfigSettings* outSetting);

	// Public properties
	//
public:
	VFrame30::SchemaDetailsSet schemasDetailsSet() const;
	std::vector<VFrame30::SchemaDetails> schemasDetails() const;
	std::set<QString> schemaAppSignals(const QString& schemaId);

	QStringList schemasByAppSignalId(const QString& appSignalId) const;
	QStringList schemasByLoopbackId(const QString& loopbackId) const;

	int configurationId() const;

	ConfigSettings configuration() const;
	QString configurationStartSchemaId() const;

	int schemaCount() const;
	QString schemaCaptionById(const QString& schemaId) const;
	QString schemaCaptionByIndex(int schemaIndex) const;
	QString schemaIdByIndex(int schemaIndex) const;

	std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> trendSchemaItems() const;

	const ComparatorSet& setpoints() const;

	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	SoftwareInfo m_softwareInfo;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	mutable QReadWriteLock m_schemaDetailsLock;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	inline static int s_configurationIdCounter = 0;

	mutable QReadWriteLock m_confugurationLock;		// for access only to m_configuration
	ConfigSettings m_configuration;

	ComparatorSet m_setpoints;
};


