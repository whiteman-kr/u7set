#pragma once

#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/ILogFile.h"
#include "TestScriptsStorage.h"

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
	QString m_ip = "0.0.0.0";
	int m_port = 0;

	friend struct ConfigSettings;
};


struct ConfigSettings
{
	int id = -1;

	bool isValid() { return id != -1; }

	int buildNo = -1;
	QString softwareEquipmentId;
	QString project;

	ConfigConnection appDataService1;
	ConfigConnection appDataService2;

	ConfigConnection appDataServiceRealtimeTrend1;
	ConfigConnection appDataServiceRealtimeTrend2;

	ConfigConnection archiveService1;
	ConfigConnection archiveService2;

	// Tuning settings
	//
	bool tuningEnabled = false;
	std::vector<TestSuiteSettings::TuningService> tuningServices;

	// Scripts list
	//
	QStringList scriptFiles;

	QString errorMessage;				// Parsing error message, empty if no errors
};


class TestSuiteConfigController : public QObject, public HasLogFile
{
	Q_OBJECT

public:
	TestSuiteConfigController() = delete;

	TestSuiteConfigController(const QString& instanceId, HostAddressPort address1, HostAddressPort address2, ILogFile *appLogFile);
	virtual ~TestSuiteConfigController();

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
	void configurationArrived();

	// slots
	//
public slots:
	void start();

private slots:
	void onConfigurationReady(const QByteArray configurationXmlData,
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
	ConfigSettings configuration() const;

	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	SoftwareInfo m_softwareInfo;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	mutable QReadWriteLock m_confugurationLock;		// for access to m_configuration
	ConfigSettings m_configuration;
};
