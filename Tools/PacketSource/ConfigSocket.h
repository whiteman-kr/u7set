#ifndef CONFIGSOCKET_H
#define CONFIGSOCKET_H

// This class is designed to receive signals from CfgSrv

#include "../../lib/CfgServerLoader.h"

#include "PacketSourceCore.h"
#include "Options.h"

// ==============================================================================================

const int				CONFIG_SOCKET_TIMEOUT_STATE		= 50;	// 50 ms

// ==============================================================================================

class ConfigSocket : public QObject
{
	Q_OBJECT

public:

	ConfigSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort,
				 PacketSourceCore* pscore);

	ConfigSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort1,
				 const HostAddressPort& serverAddressPort2,
				 PacketSourceCore* pscore);

	virtual ~ConfigSocket() override;

public:

	bool isConnceted() { return m_connected; }

	QString cfgSrvIEquipmentID() const { return m_softwareInfo.equipmentID(); }
	HostAddressPort cfgSrvIP() { return m_cfgSrvIP; }

	QString appDataSrvEquipmentID() const { return m_appDataSrvEquipmentID; }
	HostAddressPort appDataSrvIP() { return m_appDataSrvIP; }

	void start();
	void quit();

	void reconncect();

	QStringList& loadedFiles() { return m_loadedFiles; }

	QString cfgSrvInfo();
	QString appDataSrvInfo();

	E::SoftwareRunMode softwareRunMode() { return m_softwareRunMode; }

private:

	void clearConfiguration();

	SoftwareInfo m_softwareInfo;
	HostAddressPort m_serverAddressPort1;
	HostAddressPort m_serverAddressPort2;
	PacketSourceCore* m_pscore = nullptr;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	::Proto::AppSignalSet m_protoAppSignalSet;

	QTimer* m_connectionStateTimer = nullptr;
	void startConnectionStateTimer();
	void stopConnectionStateTimer();
	void updateConnectionState();

	bool m_connected = false;

	HostAddressPort m_cfgSrvIP;

	QString m_appDataSrvEquipmentID;
	HostAddressPort m_appDataSrvIP;

	QStringList m_loadedFiles;

	E::SoftwareRunMode m_softwareRunMode = E::SoftwareRunMode::Normal;

signals:

	void socketConnected();
	void socketDisconnected();

	void unknownClient();
	void unknownAdsEquipmentID(const QStringList& adsIDList);

	void configurationLoaded();
	void sourceBaseLoaded();
	void signalBaseLoading(int persentage);
	void signalBaseLoaded();

private slots:

	void slot_configurationReady(	const QByteArray configurationXmlData,
									const BuildFileInfoArray buildFileInfoArray,
									SessionParams sessionParams,
									std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	bool readConfiguration(	const QByteArray& fileData,
							std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	bool readAppSignalSet(const QByteArray& fileData);
	bool readAppDataSource(const QByteArray& fileData);

	static void loadSignalBase(ConfigSocket* pThis);
};

// ==============================================================================================

#endif // CONFIGSOCKET_H
