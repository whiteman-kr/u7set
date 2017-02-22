#include "ConfigSocket.h"

#include <assert.h>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort)
{
    HostAddressPort serverAddressPort2(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_REQUEST);

    m_cfgLoaderThread = new CfgLoaderThread(theOptions.configSocket().equipmentID(), 1, serverAddressPort, serverAddressPort2) ;
    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);
}


// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
{
    m_cfgLoaderThread = new CfgLoaderThread(theOptions.configSocket().equipmentID(), 1, serverAddressPort1,  serverAddressPort2);
    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::~ConfigSocket()
{
    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    m_cfgLoaderThread->quit();
    delete m_cfgLoaderThread;
    m_cfgLoaderThread = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::start()
{
    if (m_cfgLoaderThread == nullptr)
    {
        assert(m_cfgLoaderThread);
        return;
    }

    m_cfgLoaderThread->start();
    m_cfgLoaderThread->enableDownloadConfiguration();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
    Q_UNUSED(configurationXmlData);

    qDebug() << "ConfigSocket::slot_configurationReady - file count: " << buildFileInfoArray.count();

    for(Builder::BuildFileInfo bfi : buildFileInfoArray)
    {
        QByteArray fileData;
        QString errStr;

        qDebug() << "ConfigSocket::slot_configurationReady - file: " << bfi.pathFileName;

        m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

        if (errStr.isEmpty() == false)
        {
            qDebug() << errStr;
            continue;
        }

        // to do

    }

    emit configurationLoaded();

    return;
}

// -------------------------------------------------------------------------------------------------------------------
