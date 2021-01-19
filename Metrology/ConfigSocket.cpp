#include "ConfigSocket.h"

#include <assert.h>
#include <QtConcurrent>

#include "../lib/SoftwareSettings.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort)
	: m_softwareInfo(softwareInfo)
	, m_serverAddressPort1(serverAddressPort)
	, m_serverAddressPort2(HostAddressPort(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST))
{
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
	: m_softwareInfo(softwareInfo)
	, m_serverAddressPort1(serverAddressPort1)
	, m_serverAddressPort2(serverAddressPort2)
{
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::~ConfigSocket()
{
	quit();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::clearConfiguration()
{
	theSignalBase.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::start()
{

	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											1,
											m_serverAddressPort1,
											m_serverAddressPort2,
											false,
											nullptr);

	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);
		return;
	}

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);
	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_unknownClient, this, &ConfigSocket::unknownClient);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	startConnectionStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::quit()
{
	stopConnectionStateTimer();

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	disconnect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);

	m_cfgLoaderThread->quitAndWait();
	delete m_cfgLoaderThread;
	m_cfgLoaderThread = nullptr;

	m_connected = false;
	m_address.clear();

	emit socketDisconnected();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::reconncect(const QString& equipmentID, const HostAddressPort& serverAddressPort)
{
	if (equipmentID.isEmpty() == true)
	{
		return;
	}

	m_softwareInfo.setEquipmentID(equipmentID);
	m_serverAddressPort1 = serverAddressPort;
	m_serverAddressPort2 = HostAddressPort(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	quit();
	start();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::slot_configurationReady(const QByteArray configurationXmlData,
										   const BuildFileInfoArray buildFileInfoArray,
										   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	qDebug() << __FUNCTION__ << "File count: " << buildFileInfoArray.count();

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	m_loadedFiles.clear();
	clearConfiguration();

	bool result = false;

	result = readConfiguration(configurationXmlData, curSettingsProfile);
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

		if (bfi.ID == CfgFileId::METROLOGY_ITEMS)
		{
			result &= readMetrologyItems(fileData);					// fill MetrologyItems
		}

		if (bfi.ID == CfgFileId::METROLOGY_SIGNAL_SET)
		{
			result &= readMetrologySignalSet(fileData);				// fill MetrologySignalSet
		}

		if (bfi.ID == CfgFileId::COMPARATOR_SET)
		{
			result &= readComparatorSet(fileData);					// fill ComparatorSet
		}

		m_loadedFiles.append(bfi.pathFileName);
	}

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

	emit configurationLoaded();

	QtConcurrent::run(ConfigSocket::loadSignalBase, this);

	return;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readConfiguration(const QByteArray& fileData,
									 std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	bool result = true;

	result &= theOptions.setMetrologySettings(curSettingsProfile);
	result &= theOptions.readFromXml(fileData);

	qDebug() << __FUNCTION__ << (result == true ? "Ok" : "Error!");

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

	QElapsedTimer responseTime;
	responseTime.start();

	result &= readRacks(fileData, fileVersion);
	result &= readTuningSources(fileData, fileVersion);

	qDebug() << __FUNCTION__ << " Time for read: " << responseTime.elapsed() << " ms";

	return result;
}


// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readMetrologySignalSet(const QByteArray& fileData)
{
	QElapsedTimer responseTime;
	responseTime.start();

	bool result = m_protoMetrologySignalSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		return false;
	}

	qDebug() << __FUNCTION__ << "Signals were loaded" << theSignalBase.signalCount() << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readComparatorSet(const QByteArray& fileData)
{
	QElapsedTimer responseTime;
	responseTime.start();

	m_comparatorSet.serializeFrom(fileData);

	qDebug() << __FUNCTION__ << "Comparators were loaded" << " Time for load: " << responseTime.elapsed() << " ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readRacks(const QByteArray& fileData, int fileVersion)
{
	Q_UNUSED(fileVersion)

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
	Q_UNUSED(fileVersion)

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

		theSignalBase.tuning().sourceBase().sourceEquipmentID().append(equipmentID);
	}

	if (tuningSourceCount != theSignalBase.tuning().sourceBase().sourceEquipmentID().count())
	{
		qDebug() << __FUNCTION__ << "Tuning sources loading error, loaded: " <<
					theSignalBase.tuning().sourceBase().sourceEquipmentID().count() <<
					" from " <<
					tuningSourceCount;

		assert(false);
		return false;
	}

	qDebug() << __FUNCTION__ << "Tuning sources were loaded: " << theSignalBase.tuning().sourceBase().sourceEquipmentID().count();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::loadSignalBase(ConfigSocket* pThis)
{
	if (pThis == nullptr)
	{
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	int persentage = 0;

	int signalCount = pThis->m_protoMetrologySignalSet.metrologysignal_size();
	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::MetrologySignal& protoAppSignal = pThis->m_protoMetrologySignalSet.metrologysignal(i);

		Metrology::SignalParam param;
		param.serializeFrom(protoAppSignal);

		theSignalBase.appendSignal(param);

		persentage = i * 100 / signalCount;

		if (persentage % 5 == 0)
		{
			emit pThis->signalBaseLoading(persentage);
		}
	}

	theSignalBase.initSignals();

	theSignalBase.loadComparatorsInSignal(pThis->m_comparatorSet);

	emit pThis->signalBaseLoaded();

	qDebug() << __FUNCTION__ << " Time for load: " << responseTime.elapsed() << " ms";
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

	Tcp::ConnectionState&& state = m_cfgLoaderThread->getConnectionState();

	if (state.isConnected != m_connected)
	{
		m_connected = state.isConnected;

		if (m_connected == true)
		{
			m_address = state.peerAddr;

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
