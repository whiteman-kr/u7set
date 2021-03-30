#pragma once

#include "../lib/CfgServerLoader.h"
#include "../lib/SocketIO.h"
#include "../lib/ComparatorSet.h"
#include "../lib/ClientBehavior.h"
#include "../VFrame30/Schema.h"


class ConfigConnection
{
public:
	ConfigConnection() = default;
	ConfigConnection(const ConfigConnection&) = default;
	ConfigConnection(QString EquipmentId, QString ipAddress, int port);
	ConfigConnection& operator=(const ConfigConnection&) = default;

	QString equipmentId() const;
	QString ip() const;
	int port() const;

	HostAddressPort address() const;

protected:
	QString m_equipmentId = "UNKNOWN";
	QString m_ip = "127.0.0.1";
	int m_port = 0;

	friend struct ConfigSettings;
};


struct ConfigSettings
{
	int buildNo = -1;
	QString softwareEquipmentId;
	QString project;
	QString startSchemaId;

	ConfigConnection appDataService1;
	ConfigConnection appDataService2;

	ConfigConnection appDataServiceRealtimeTrend1;
	ConfigConnection appDataServiceRealtimeTrend2;

	ConfigConnection archiveService1;
	ConfigConnection archiveService2;

	bool tuningEnabled = false;
	QStringList tuningSources;
	ConfigConnection tuningService;

	QString globalScript;
	QString onConfigurationArrivedScript;
	QImage logoImage;

	MonitorBehavior monitorBeahvior;

	QString errorMessage;				// Parsing error message, empty if no errors
};


class MonitorConfigController : public QObject
{
	Q_OBJECT

public:
	MonitorConfigController() = delete;

	MonitorConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2);
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
	void unknownClient();										// Error if CfgService cannot find SoftwareID

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

	ConfigSettings configuration() const;
	QString configurationStartSchemaId() const;

	int schemaCount() const;
	QString schemaCaptionById(const QString& schemaId) const;
	QString schemaCaptionByIndex(int schemaIndex) const;
	QString schemaIdByIndex(int schemaIndex) const;

	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	SoftwareInfo m_softwareInfo;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	mutable QMutex m_mutex;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;

	mutable QMutex m_confugurationMutex;		// for access only to m_configuration
	ConfigSettings m_configuration;
};


