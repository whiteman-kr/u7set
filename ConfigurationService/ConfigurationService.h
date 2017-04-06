#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class declaration
//
// ------------------------------------------------------------------------------------

class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ConfigurationServiceWorker(const QString& serviceName, int& argc, char** argv, const VersionInfo& versionInfo);

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const;

public slots:
	void onInformationRequest(UdpRequest request);

signals:
	void ackInformationRequest(UdpRequest request);

private:
	virtual void initCmdLineParser() override;
	virtual void processCmdLineSettings() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void startCfgServerThread();
	void stopCfgServerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

private:
	UdpSocketThread* m_infoSocketThread = nullptr;
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	// settings
	//
	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_clientIPStr;
	QString m_workDirectory;

	HostAddressPort m_clientIP;
};


// ------------------------------------------------------------------------------------
//
// ListenerWithLog class declaration
//
// ------------------------------------------------------------------------------------

class ListenerWithLog : public Tcp::Listener
{
public:
	ListenerWithLog(const HostAddressPort& listenAddressPort, Tcp::Server* server);

	virtual void onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr) override;
};


// ------------------------------------------------------------------------------------
//
// CfgServerWithLog class declaration
//
// ------------------------------------------------------------------------------------

class CfgServerWithLog : public CfgServer
{
public:
	CfgServerWithLog(const QString& buildFolder);

	virtual CfgServer* getNewInstance() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onFileSent(const QString& fileName) override;
};


