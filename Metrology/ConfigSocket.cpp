#include "ConfigSocket.h"

#include <assert.h>

#include "Options.h"

#include "../lib/ServiceSettings.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort)
{
	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY);
	if (equipmentID.isEmpty() == true)
	{
		return;
	}

	HostAddressPort serverAddressPort2(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_REQUEST);

	m_cfgLoaderThread = new CfgLoaderThread(equipmentID, 1, serverAddressPort, serverAddressPort2) ;
    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);

	startConnectionStateTimer();
}


// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
{
	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY);
	if (equipmentID.isEmpty() == true)
	{
		return;
	}

	m_cfgLoaderThread = new CfgLoaderThread(equipmentID, 1, serverAddressPort1,  serverAddressPort2);
    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);

	startConnectionStateTimer();
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

	stopConnectionStateTimer();
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
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
    qDebug() << "ConfigSocket::slot_configurationReady - file count: " << buildFileInfoArray.count();

	if (m_cfgLoaderThread == nullptr)
    {
        return;
	}

	m_loadedFiles.clear();

    //clearConfiguration();

    bool result = readConfiguration(configurationXmlData);

    if (result == false)
    {
        return;
    }

    for(Builder::BuildFileInfo bfi : buildFileInfoArray)
    {
        QByteArray fileData;
        QString errStr;

        m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

        if (errStr.isEmpty() == false)
        {
            qDebug() << errStr;
            continue;
        }

		result = true;

		if (bfi.ID == CFG_FILE_ID_METROLOGY_SIGNALS)
        {
            result &= readMetrologySignals(fileData);				// fill Units and MetrologySignals
        }

		m_loadedFiles.append(bfi.pathFileName);
    }

    emit configurationLoaded();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readConfiguration(const QByteArray& fileData)
{
    XmlReadHelper xml(fileData);

	bool result = theOptions.readFromXml(xml);

	if (result == true)
	{
		qDebug() << "ConfigSocket::readConfiguration - OK";
	}
	else
	{
		qDebug() << "ConfigSocket::readConfiguration - ERROR!";
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readMetrologySignals(QByteArray& fileData)
{
    bool result = true;

    XmlReadHelper xml(fileData);

	if (xml.findElement("Units") == false)
	{
		return false;
	}

	int unitCount = 0;

	result &= xml.readIntAttribute("Count", &unitCount);

	for(int count = 0; count < unitCount; count++)
	{
		if(xml.findElement("Unit") == false)
		{
			result = false;
			break;
		}

		int unitID = 0;
		QString unitCaption;

		result &= xml.readIntAttribute("ID", &unitID);
		result &= xml.readStringAttribute("Caption", &unitCaption);

		// theUnitBase.appendUnit(unitID, unitCaption);
		// m_unitInfo.append(unitID, unitCaption);
	}


//    if (theUnitBase.unitCount() != unitCount)
//    if (m_unitInfo.count() != unitCount)
//    {
//        qDebug() << "Units loading error";
//        assert(false);
//        return false;
//    }

	if (xml.findElement("Signals") == false)
	{
		return false;
	}

	int signalCount = 0;

	result &= xml.readIntAttribute("Count", &signalCount);

	for(int count = 0; count < signalCount; count++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

		SignalParam param;

		bool res = param.readFromXml(xml);	// time-expensive function !!!

		if (res == true)
		{
//            if (m_appSignals.contains(signal->appSignalID()) == false)
//            {
//                m_appSignals.insert(signal->appSignalID(), signal);
//            }
//            else
//            {
//                res = false;
//            }
		}

		result &= res;
	}

	//m_appSignals.buildHash2Signal();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::startConnectionStateTimer()
{
	if (m_connectionStateTimer == nullptr)
	{
		m_connectionStateTimer = new QTimer(this);
		connect(m_connectionStateTimer, &QTimer::timeout, this, &ConfigSocket::updateConnectionState);
	}

	m_connectionStateTimer->start(CONFIG_SOCKET_TIMEOUT_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::stopConnectionStateTimer()
{
	if (m_connectionStateTimer != nullptr)
	{
		m_connectionStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::updateConnectionState()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	if (m_cfgLoaderThread->getConnectionState().isConnected != m_connected)
	{
		m_connected = m_cfgLoaderThread->getConnectionState().isConnected;

		if (m_connected == true)
		{
			m_address = m_cfgLoaderThread->getConnectionState().peerAddr;

			emit socketConnected();
		}
		else
		{
			m_address.clear();

			emit socketDisconnected();
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
