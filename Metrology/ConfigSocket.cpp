#include "ConfigSocket.h"

#include <assert.h>

#include "Options.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"

#include "../lib/ServiceSettings.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort)
{
	// use only SOCKET_SERVER_TYPE_PRIMARY
	//
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
	// use only SOCKET_SERVER_TYPE_PRIMARY
	//
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

void ConfigSocket::clearConfiguration()
{
	m_loadedFiles.clear();

	theUnitBase.clear();
	theSignalBase.clear();

	theTuningSignalBase.createSignalList();
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

	clearConfiguration();

	bool result = false;

	result = readConfiguration(configurationXmlData);
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
	bool result = theOptions.readFromXml(fileData);

	qDebug() << "ConfigSocket::readConfiguration - " << (result == true ? "OK" : "ERROR!");

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readMetrologySignals(const QByteArray& fileData)
{
    bool result = true;

    XmlReadHelper xml(fileData);

	// load units
	//

	if (xml.findElement("Units") == false)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - Units section not found";
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

		theUnitBase.appendUnit(unitID, unitCaption);
	}

	if (theUnitBase.unitCount() != unitCount)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - Units loading error, loaded: " << theUnitBase.unitCount() << " from " << unitCount;
		assert(false);
		return false;
	}

	qDebug() << "ConfigSocket::readMetrologySignals - Units were loaded:	" << theUnitBase.unitCount();

	// load signals
	//

	if (xml.findElement("Signals") == false)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - Signals section not found";
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

		bool res = param.readFromXml(xml);
		if (res == true)
		{
			if (theSignalBase.appendSignal(param) == -1)
			{
				res = false;
			}
		}

		result &= res;
	}

	if (theSignalBase.signalCount() != signalCount)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - Signals loading error, loaded: " << theSignalBase.signalCount() << " from " << signalCount;
		assert(false);
		return false;
	}

	theSignalBase.sortByPosition();
	theSignalBase.setCaseNoForAllSignals();

	theTuningSignalBase.createSignalList();

	qDebug() << "ConfigSocket::readMetrologySignals - Signals were loaded:	" << theSignalBase.signalCount();

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
