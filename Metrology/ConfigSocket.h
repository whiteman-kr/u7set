#ifndef CONFIGSOCKET_H
#define CONFIGSOCKET_H

// This class is designed to receive signals from CfgSrv
//
// Algorithm:
//

#include "../lib/CfgServerLoader.h"

// ==============================================================================================

const int				CONFIG_SOCKET_TIMEOUT_STATE		= 50;	// 50 ms

// ==============================================================================================

class ConfigSocket : public QObject
{
	Q_OBJECT

public:

	ConfigSocket(const HostAddressPort& serverAddressPort);
	ConfigSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~ConfigSocket();

private:

	CfgLoaderThread*	m_cfgLoaderThread = nullptr;

	void				clearConfiguration();

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

	QStringList&		loadedFiles() { return m_loadedFiles; }

private slots:

	void				slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	bool				readConfiguration(const QByteArray& fileData);
	bool				readMetrologySignals(const QByteArray& fileData);

	bool				readRacks(const QByteArray& fileData, int fileVersion);
	bool				readTuningSources(const QByteArray& fileData, int fileVersion);
	bool				readUnits(const QByteArray& fileData, int fileVersion);
	bool				readSignals(const QByteArray& fileData, int fileVersion);

signals:

	void				socketConnected();
	void				socketDisconnected();

	void				configurationLoaded();
};

// ==============================================================================================

#endif // CONFIGSOCKET_H
