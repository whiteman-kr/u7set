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
	qDebug() << __FUNCTION__ << "File count: " << buildFileInfoArray.count();

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

		if (bfi.ID == CFG_FILE_ID_METROLOGY_ITEMS)
		{
			result &= readMetrologyItems(fileData);					// fill MetrologyItems
		}

		if (bfi.ID == CFG_FILE_ID_METROLOGY_SIGNAL_SET)
		{
			result &= readMetrologySignalSet(fileData);				// fill MetrologySignalSet
		}

		if (bfi.ID == CFG_FILE_ID_COMPARATOR_SET)
		{
			result &= readComparatorSet(fileData);					// fill ComparatorSet
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

	qDebug() << __FUNCTION__ << (result == true ? "OK" : "ERROR!");

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readMetrologyItems(const QByteArray& fileData)
{
	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("MetrologyItems") == false)
	{
		qDebug() << __FUNCTION__ << "Section Version not found";
		return false;
	}

	int fileVersion = 0;
	result &= xml.readIntAttribute("Version", &fileVersion);
	if (result == false || fileVersion == 0)
	{
		qDebug() << __FUNCTION__ << "File version undefined";
		return false;
	}

	theOptions.projectInfo().setCfgFileVersion(fileVersion);

	if (fileVersion != CFG_FILE_VER_METROLOGY_ITEMS_XML)
	{
		qDebug() << __FUNCTION__ << "Failed fileVersion, waited:" << CFG_FILE_VER_METROLOGY_ITEMS_XML << ", recieved:" << fileVersion;
		return false;
	}

	QTime responseTime;
	responseTime.start();

	result &= readRacks(fileData, fileVersion);
	result &= readTuningSources(fileData, fileVersion);

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

	return result;
}


// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readMetrologySignalSet(const QByteArray& fileData)
{
	::Proto::MetrologySignalSet protoMetrologySignalSet;

	QTime responseTime;
	responseTime.start();

	bool result = protoMetrologySignalSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		return false;
	}

	int signalCount = protoMetrologySignalSet.metrologysignal_size();
	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::MetrologySignal& protoAppSignal = protoMetrologySignalSet.metrologysignal(i);

		Metrology::SignalParam param;
		param.serializeFrom(protoAppSignal);

		theSignalBase.appendSignal(param);
	}

	theSignalBase.initSignals();

	qDebug() << __FUNCTION__ << "Signals were loaded" << theSignalBase.signalCount() << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readComparatorSet(const QByteArray& fileData)
{
	QTime responseTime;
	responseTime.start();

	ComparatorSet comparatorSet;
	comparatorSet.serializeFrom(fileData);

	theSignalBase.loadComparatorsInSignal(comparatorSet);

	qDebug() << __FUNCTION__ << "Comparators were loaded" << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readRacks(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion);

	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("Racks") == false)
	{
		qDebug() << __FUNCTION__ << "Racks section not found";
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
		qDebug() << __FUNCTION__ << "Racks loading error, loaded: " << theSignalBase.racks().count() << " from " << rackCount;
		assert(false);
		return false;
	}

	qDebug() << __FUNCTION__ << "Racks were loaded: " << theSignalBase.racks().count();

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
		qDebug() << __FUNCTION__ << "TuningSources section not found";
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
		qDebug() << __FUNCTION__ << "Tuning sources loading error, loaded: " << theSignalBase.tuning().Sources().sourceEquipmentID().count() << " from " << tuningSourceCount;
		assert(false);
		return false;
	}

	qDebug() << __FUNCTION__ << "Tuning sources were loaded: " << theSignalBase.tuning().Sources().sourceEquipmentID().count();

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
