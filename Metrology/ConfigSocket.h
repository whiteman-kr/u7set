#ifndef CONFIGSOCKET_H
#define CONFIGSOCKET_H

// This class is designed to receive signals from CfgSrv
//
// Algorithm:
//

#include "../lib/CfgServerLoader.h"

// ==============================================================================================

class ConfigSocket : public QObject
{
     Q_OBJECT

public:
                        ConfigSocket(const HostAddressPort& serverAddressPort);
                        ConfigSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
                        ~ConfigSocket();

    void                start();

private:

    CfgLoaderThread*    m_cfgLoaderThread = nullptr;

private slots:

    void                slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

    bool                readConfiguration(const QByteArray& fileData);
    bool                readMetrologySignals(QByteArray& fileData);

signals:

    void                configurationLoaded();
};

// ==============================================================================================

#endif // CONFIGSOCKET_H
