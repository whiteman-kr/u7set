#ifndef MONITORCONFIGTHREAD_H
#define MONITORCONFIGTHREAD_H

#include "../lib/CfgServerLoader.h"
#include "../lib/SocketIO.h"
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

	Tcp::ConnectionState getConnectionState() const;

	const SoftwareInfo& softwareInfo() const;

	// signals
	//
signals:
	void configurationArrived(ConfigSettings configuration);

	// slots
	//
public slots:
	void start();

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	bool xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigSettings* outSetting);
	bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting);
	bool xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting);

	// Public properties
	//
public:
	std::vector<VFrame30::SchemaDetails> schemasDetails() const;
	std::set<QString> schemaAppSignals(const QString& schemaId);

	ConfigSettings configuration() const;

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

#endif // MONITORCONFIGTHREAD_H
