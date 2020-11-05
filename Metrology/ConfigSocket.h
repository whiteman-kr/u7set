#ifndef CONFIGSOCKET_H
#define CONFIGSOCKET_H

// This class is designed to receive signals from CfgSrv

#include "../lib/CfgServerLoader.h"

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

	virtual ~ConfigSocket();

private:

	void				clearConfiguration();

	SoftwareInfo		m_softwareInfo;
	HostAddressPort		m_serverAddressPort1;
	HostAddressPort		m_serverAddressPort2;


	CfgLoaderThread*	m_cfgLoaderThread = nullptr;

	QTimer*				m_connectionStateTimer = nullptr;
	void				startConnectionStateTimer();
	void				stopConnectionStateTimer();
	void				updateConnectionState();

	bool				m_connected = false;
	HostAddressPort		m_address;

	QStringList			m_loadedFiles;

public:

	bool				isConnceted() { return m_connected; }
	HostAddressPort		address() { return m_address; }

	void				start();
	void				quit();

	void				reconncect(const QString& equipmentID, const HostAddressPort& serverAddressPort);

	QStringList&		loadedFiles() { return m_loadedFiles; }

private slots:

	void				slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	bool				readConfiguration(const QByteArray& fileData);
	bool				readMetrologyItems(const QByteArray& fileData);
	bool				readMetrologySignalSet(const QByteArray& fileData);
	bool				readComparatorSet(const QByteArray& fileData);

	bool				readRacks(const QByteArray& fileData, int fileVersion);
	bool				readTuningSources(const QByteArray& fileData, int fileVersion);

signals:

	void				socketConnected();
	void				socketDisconnected();

	void				configurationLoaded();
};

// ==============================================================================================

#endif // CONFIGSOCKET_H
