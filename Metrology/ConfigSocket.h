#ifndef CONFIGSOCKET_H
#define CONFIGSOCKET_H

// This class is designed to receive signals from CfgSrv
//
// Algorithm:
//

#include "../lib/CfgServerLoader.h"

// ==============================================================================================

const int				CONFIG_SOCKET_TIMEOUT_STATE    = 50;  // 50 ms

// ==============================================================================================

class ConfigSocket : public QObject
{
     Q_OBJECT

public:
                        ConfigSocket(const HostAddressPort& serverAddressPort);
                        ConfigSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
                        ~ConfigSocket();



private:

	CfgLoaderThread*	m_cfgLoaderThread = nullptr;

	QTimer*				m_connectionStateTimer = nullptr;
	void				startConnectionStateTimer();
	void				stopConnectionStateTimer();
	void				updateConnectionState();

	bool				m_connected = false;
	HostAddressPort		m_address;

	QVector<QString>	m_loadedFiles;

public:

	bool				isConnceted() { return m_connected; }
	HostAddressPort		address() { return m_address; }

	void                start();

	int					loadedFilesCount() { return m_loadedFiles.count(); }
	QString				loadedFile(int index) { if (index < 0 || index >= m_loadedFiles.count()) { return QString(); } return m_loadedFiles[index]; }

private slots:

    void                slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

    bool                readConfiguration(const QByteArray& fileData);
    bool                readMetrologySignals(QByteArray& fileData);

signals:

	void				socketConnected();
	void				socketDisconnected();

    void                configurationLoaded();
};

// ==============================================================================================

#endif // CONFIGSOCKET_H
