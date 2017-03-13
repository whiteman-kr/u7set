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

	if (xml.findElement("MetrologySignals") == false)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - section Version not found";
		return false;
	}

	int fileVersion = 0;
	result &= xml.readIntAttribute("Version", &fileVersion);
	if (result == false || fileVersion == 0)
	{
		qDebug() << "ConfigSocket::readMetrologySignals - file version undefined";
		return false;
	}

	result &= readRacks(fileData, fileVersion);
	result &= readUnits(fileData, fileVersion);
	result &= readSignals(fileData, fileVersion);

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readRacks(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion);

	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("Racks") == false)
	{
		qDebug() << "ConfigSocket::readRacks - Racks section not found";
		return false;
	}

	Metrology::RackParam rack;

	int racksCount = 0;
	result &= xml.readIntAttribute("Count", &racksCount);

	for(int r = 0; r < racksCount; r++)
	{
		if(xml.findElement("Rack") == false)
		{
			result = false;
			break;
		}

		result &= rack.readFromXml(xml);
		if (result == false)
		{
			continue;
		}

		theRackBase.append(rack);
	}

	if (theRackBase.count() != racksCount)
	{
		qDebug() << "ConfigSocket::readRacks - Racks loading error, loaded: " << theRackBase.count() << " from " << racksCount;
		assert(false);
		return false;
	}

	qDebug() << "ConfigSocket::readRacks - Racks were loaded:	" << theRackBase.count();

	theRackBase.updateParamFromGroups();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readUnits(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion);

	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("Units") == false)
	{
		qDebug() << "ConfigSocket::readUnits - Units section not found";
		return false;
	}

	int unitsCount = 0;
	result &= xml.readIntAttribute("Count", &unitsCount);

	for(int u = 0; u < unitsCount; u++)
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

	if (theUnitBase.unitCount() != unitsCount)
	{
		qDebug() << "ConfigSocket::readUnits - Units loading error, loaded: " << theUnitBase.unitCount() << " from " << unitsCount;
		assert(false);
		return false;
	}

	qDebug() << "ConfigSocket::readUnits - Units were loaded:	" << theUnitBase.unitCount();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readSignals(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion);

	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("Signals") == false)
	{
		qDebug() << "ConfigSocket::readSignals - Signals section not found";
		return false;
	}

	Metrology::SignalParam param;

	int signalsCount = 0;
	result &= xml.readIntAttribute("Count", &signalsCount);

	for(int s = 0; s < signalsCount; s++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

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

	if (theSignalBase.signalCount() != signalsCount)
	{
		qDebug() << "ConfigSocket::readSignals- Signals loading error, loaded: " << theSignalBase.signalCount() << " from " << signalsCount;
		assert(false);
		return false;
	}

	theSignalBase.initSignals();

	theTuningSignalBase.createSignalList();

	qDebug() << "ConfigSocket::readSignals - Signals were loaded:	" << theSignalBase.signalCount();

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
