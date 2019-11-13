#include "ConfigSocket.h"

#include <assert.h>

#include "Options.h"
#include "SignalBase.h"

#include "../lib/ServiceSettings.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort, const SoftwareInfo& softwareInfo)
{
	// use only SOCKET_SERVER_TYPE_PRIMARY
	//
	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY);
	if (equipmentID.isEmpty() == true)
	{
		return;
	}

	HostAddressPort serverAddressPort2(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo,
											1,
											serverAddressPort,
											serverAddressPort2,
											false,
											nullptr);

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);

	startConnectionStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2,
						   const SoftwareInfo& softwareInfo)
{
	// use only SOCKET_SERVER_TYPE_PRIMARY
	//
	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY);

	if (equipmentID.isEmpty() == true)
	{
		return;
	}

	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo,
											1,
											serverAddressPort1,
											serverAddressPort2,
											false,
											nullptr);

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

	theSignalBase.clear();
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

	QTime responseTime;
	responseTime.start();

	clearConfiguration();

	bool result = false;

	result = readConfiguration(configurationXmlData);
	if (result == false)
	{
		return;
	}

	for(const Builder::BuildFileInfo& bfi : buildFileInfoArray)
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

		if (bfi.ID == CFG_FILE_ID_APP_SIGNAL_SET)
		{
			result &= readAppSignalSet(fileData);					// fill AppSignalSet
		}

		if (bfi.ID == CFG_FILE_ID_COMPARATOR_SET)
		{
			result &= readComparatorSet(fileData);					// fill ComparatorSet
		}

		if (bfi.ID == CFG_FILE_ID_METROLOGY_SIGNALS)
		{
			result &= readMetrologySignals(fileData);				// fill MetrologySignals
		}

		m_loadedFiles.append(bfi.pathFileName);
	}

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

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

bool ConfigSocket::readAppSignalSet(const QByteArray& fileData)
{
	::Proto::AppSignalSet protoAppSignalSet;

	QTime responseTime;
	responseTime.start();

	bool result = protoAppSignalSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		return false;
	}

	int signalCount = protoAppSignalSet.appsignal_size();
	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

		Metrology::SignalParam param;
		param.serializeFrom(protoAppSignal);

		theSignalBase.appendSignal(param);
	}

	qDebug() << __FUNCTION__ << "Signals were loaded" << theSignalBase.signalCount() << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readComparatorSet(const QByteArray& fileData)
{
	::Proto::ComparatorSet protoComparatorSet;

	QTime responseTime;
	responseTime.start();

	bool result = protoComparatorSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		return false;
	}

	Builder::ComparatorSet comparatorSet;
	comparatorSet.serializeFrom(protoComparatorSet);

	theSignalBase.loadComparators(comparatorSet);

	qDebug() << __FUNCTION__ << "Comparators were loaded" << comparatorSet.comparatorCount() << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
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

	theOptions.projectInfo().setCfgFileVersion(fileVersion);

	if (fileVersion != CFG_FILE_VER_METROLOGY_SIGNALS)
	{
		qDebug() << tr("ConfigSocket::readMetrologySignals - failed fileVersion, waited:") << CFG_FILE_VER_METROLOGY_SIGNALS << tr(", recieved:") << fileVersion;
		return false;
	}

	QTime responseTime;
	responseTime.start();

	result &= readRacks(fileData, fileVersion);
	result &= readTuningSources(fileData, fileVersion);
	result &= readSignals(fileData, fileVersion);

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

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

	int rackCount = 0;
	result &= xml.readIntAttribute("Count", &rackCount);

	for(int r = 0; r < rackCount; r++)
	{
		if (xml.findElement("Rack") == false)
		{
			result = false;
			break;
		}

		result &= rack.readFromXml(xml);
		if (result == false)
		{
			continue;
		}

		theSignalBase.racks().append(rack);
	}

	if (theSignalBase.racks().count() != rackCount)
	{
		qDebug() << "ConfigSocket::readRacks - Racks loading error, loaded: " << theSignalBase.racks().count() << " from " << rackCount;
		assert(false);
		return false;
	}

	qDebug() << "ConfigSocket::readRacks - Racks were loaded: " << theSignalBase.racks().count();

	theSignalBase.racks().updateParamFromGroups();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readTuningSources(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion);

	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("TuningSources") == false)
	{
		qDebug() << "ConfigSocket::readTuningSources - TuningSources section not found";
		return false;
	}

	int tuningSourceCount = 0;
	result &= xml.readIntAttribute("Count", &tuningSourceCount);

	for(int t = 0; t < tuningSourceCount; t++)
	{
		if (xml.findElement("TuningSource") == false)
		{
			result = false;
			break;
		}

		QString equipmentID;

		result &= xml.readStringAttribute("EquipmentID", &equipmentID);

		theSignalBase.tuning().Sources().sourceEquipmentID().append(equipmentID);
	}

	if (tuningSourceCount != theSignalBase.tuning().Sources().sourceEquipmentID().count())
	{
		qDebug() << "ConfigSocket::readTuningSources - Tuning sources loading error, loaded: " << theSignalBase.tuning().Sources().sourceEquipmentID().count() << " from " << tuningSourceCount;
		assert(false);
		return false;
	}

	qDebug() << "ConfigSocket::readTuningSources - Tuning sources were loaded: " << theSignalBase.tuning().Sources().sourceEquipmentID().count();

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

	QTime responseTime;
	responseTime.start();

	Metrology::SignalParam param;

	int signalCount = 0;
	result &= xml.readIntAttribute("Count", &signalCount);

	for(int s = 0; s < signalCount; s++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

		bool res = param.readFromXml(xml);
		if (res == true)
		{
			Metrology::Signal* pSignal = theSignalBase.signalPtr(param.location().appSignalID());
			if (pSignal == nullptr)
			{
				continue;
			}

			pSignal->param().updateParam(param);
		}

		result &= res;
	}

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

	theSignalBase.initSignals();

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
