#pragma once

// This class is designed to receive signals from CfgSrv

#include "../OnlineLib/CfgServerLoader.h"

#include "Options.h"

// ==============================================================================================

const int				CONFIG_SOCKET_TIMEOUT_STATE		= 50;	// 50 ms

// ==============================================================================================

class ConfigSocket : public QObject
{
	Q_OBJECT

public:

	ConfigSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort);

	ConfigSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort1,
				 const HostAddressPort& serverAddressPort2);

	virtual ~ConfigSocket() override;

public:

	bool				isConnceted() { return m_connected; }
	HostAddressPort		address() { return m_address; }

	void				start();
	void				quit();

	void				reconncect(const QString& equipmentID, const HostAddressPort& serverAddressPort);

	QStringList&		loadedFiles() { return m_loadedFiles; }

private:

	void				clearConfiguration();

	SoftwareInfo		m_softwareInfo;
	HostAddressPort		m_serverAddressPort1;
	HostAddressPort		m_serverAddressPort2;
	SocketClientOption	m_option;


	CfgLoaderThread*	m_cfgLoaderThread = nullptr;

	::Proto::MetrologySignalSet m_protoMetrologySignalSet;
	ComparatorSet		m_comparatorSet;

	QTimer*				m_connectionStateTimer = nullptr;
	void				startConnectionStateTimer();
	void				stopConnectionStateTimer();
	void				updateConnectionState();

	bool				m_connected = false;
	HostAddressPort		m_address;

	QStringList			m_loadedFiles;

private slots:

	void				slot_configurationReady(const QByteArray configurationXmlData,
												const BuildFileInfoArray buildFileInfoArray,
												SessionParams sessionParams,
												std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	bool				readConfiguration(const QByteArray& fileData,
										  std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	bool				readMetrologyItems(const QByteArray& fileData);
	bool				readMetrologySignalSet(const QByteArray& fileData);
	bool				readComparatorSet(const QByteArray& fileData);

	bool				readRacks(const QByteArray& fileData, int fileVersion);
	bool				readMetrologyConnections(const QByteArray& fileData, int fileVersion);
	bool				readTuningSources(const QByteArray& fileData, int fileVersion);

	static void			loadSignalBase(ConfigSocket* pThis);

signals:

	void				socketConnected();
	void				socketDisconnected();

	void				unknownClient();
	void				configurationLoaded();
	void				signalBaseLoading(int persentage);
	void				signalBaseLoaded();
};

// ==============================================================================================
