#include "SimAppDataTransmitter.h"
#include "Simulator.h"

#include "../lib/LogicModulesInfo.h"
#include "../OnlineLib/DataProtocols.h"

namespace Sim
{

	AppDataTransmitter::AppDataTransmitter(Simulator* simulator) :
		m_simulator(simulator),
		m_log(m_simulator->log(), "AppDataTransmitter")
	{
		connect(m_simulator, &Simulator::projectUpdated, this, &AppDataTransmitter::projectUpdated);
	}

	AppDataTransmitter::~AppDataTransmitter()
	{
		shutdownTransmitterThread();
	}

	bool AppDataTransmitter::startSimulation(QString profileName)
	{
		TEST_PTR_RETURN_FALSE(m_simulator);

		m_transmitterThread = new AppDataTransmitterThread(*m_simulator, profileName, m_log);
		m_transmitterThread->start();

		return true;
	}

	bool AppDataTransmitter::stopSimulation()
	{
		shutdownTransmitterThread();

		return true;
	}

	bool AppDataTransmitter::sendData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp)
	{
		if (softwareEnabled() == false)
		{
			return true;
		}

		TEST_PTR_RETURN_FALSE(m_transmitterThread);

		return m_transmitterThread->sendAppData(lmEquipmentId, portEquipmentId, data, timeStamp);
	}

	void AppDataTransmitter::projectUpdated()
	{
		// Project was loaded or cleared
		// Reset all queues here
		//
	}

	bool AppDataTransmitter::softwareEnabled() const
	{
		return m_simulator->software().enabled();
	}

	void AppDataTransmitter::shutdownTransmitterThread()
	{
		if (m_transmitterThread != nullptr)
		{
			m_transmitterThread->quitAndWait();
			delete m_transmitterThread;
			m_transmitterThread = nullptr;
		}
	}

	// --------------------------------------------------------------------------------------------------
	//
	// class AppDataTransmitterThread
	//
	// --------------------------------------------------------------------------------------------------

	AppDataTransmitterThread::AppDataTransmitterThread(const Simulator& simulator,
													   const QString& curProfileName,
													   ScopedLog& log) :
		m_simulator(simulator),
		m_curProfileName(curProfileName),
		m_log(log)
	{
	}

	AppDataTransmitterThread::~AppDataTransmitterThread()
	{
	}

	bool AppDataTransmitterThread::sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp)
	{
		QThread* curThread = QThread::currentThread();

		m_appDataQueueMutex.lock(curThread);

		ExtAppData& extAppData = m_appDataQueue.emplace();

		extAppData.lmEquipmentID = lmEquipmentId;
		extAppData.appData = data;
		extAppData.portEquipmentID = portEquipmentId;
		extAppData.timeStamp = timeStamp;

		m_appDataQueueMutex.unlock(curThread);

		return true;
	}

	void AppDataTransmitterThread::run()
	{
		initAppDataSources();

		m_socket = new QUdpSocket();

		QThread* curThread = QThread::currentThread();

		ExtAppData extAppData;

		while(isQuitRequested() == false)
		{
			m_appDataQueueMutex.lock(curThread);

			if (m_appDataQueue.empty() == false)
			{
				ExtAppData& queueAppData = m_appDataQueue.front();

				extAppData.lmEquipmentID = queueAppData.lmEquipmentID;
				extAppData.portEquipmentID = queueAppData.portEquipmentID;
				extAppData.timeStamp = queueAppData.timeStamp;
				extAppData.appData.swap(queueAppData.appData);

				m_appDataQueue.pop();

				m_appDataQueueMutex.unlock(curThread);

				privateSendAppData(extAppData);
			}
			else
			{
				m_appDataQueueMutex.unlock(curThread);
				msleep(1);
			}
		}

		delete m_socket;
		m_socket = nullptr;
	}

	void AppDataTransmitterThread::initAppDataSources()
	{
		std::vector<std::shared_ptr<LogicModule>> logicModules = m_simulator.logicModules();

		for(std::shared_ptr<LogicModule> logicModule : logicModules)
		{
			TEST_PTR_CONTINUE(logicModule);

			::LogicModuleInfo lmi = logicModule->logicModuleExtraInfo();

			if (lmi.appDataEnable == false)
			{
				continue;
			}

			for(LanControllerInfo lci : lmi.lanControllers)
			{
				if (lci.appDataProvided == true && lci.appDataEnable == true)
				{
					AppDataSourcePortInfo adspi;

					adspi.equipmentID = lci.equipmentID;

					adspi.appDataUID = lmi.appDataUID;
					adspi.appDataSizeBytes = lmi.appDataSizeBytes;

					adspi.moduleType = lmi.moduleType();

					adspi.rupFramesCount = (adspi.appDataSizeBytes / sizeof(Rup::Data)) +
											((adspi.appDataSizeBytes % sizeof(Rup::Data)) == 0 ? 0 : 1);

					adspi.lanSourceIP = QHostAddress(lci.appDataIP);
					adspi.lanSourcePort = lci.appDataPort;

					std::shared_ptr<const AppDataServiceSettings> settings =
							m_simulator.software().getSettingsProfile<AppDataServiceSettings>(lci.appDataServiceID, m_curProfileName);

					if (settings == nullptr)
					{
						m_log.writeError(QString("Settings profile '%1' is not found for AppDataService %2").
														arg(m_curProfileName).arg(lci.appDataServiceID));
						continue;
					}

					adspi.lanDestinationIP = settings->appDataReceivingIP.address();
					adspi.lanDestinationPort = settings->appDataReceivingIP.port();

					m_appDataSourcePorts.insert({lci.equipmentID, adspi});
				}
			}
		}
	}

	void AppDataTransmitterThread::privateSendAppData(const ExtAppData& extAppData)
	{
		auto item = m_appDataSourcePorts.find(extAppData.portEquipmentID);

		if (item == m_appDataSourcePorts.end())
		{
			return;
		}

		AppDataSourcePortInfo& adspi = item->second;

		Q_ASSERT(extAppData.appData.size() == adspi.appDataSizeBytes);

		Rup::SimFrame simFrame;

		simFrame.simVersion = reverseUint16(1);

		Rup::Frame& rupFrame = simFrame.rupFrame;

		Rup::Header& rupHeader = rupFrame.header;

		rupHeader.frameSize = sizeof(rupFrame);
		rupHeader.protocolVersion = 5;

		rupHeader.flags.all = 0;
		rupHeader.flags.appData = 1;

		rupHeader.dataId = adspi.appDataUID;
		rupHeader.moduleType = static_cast<quint16>(adspi.moduleType);
		rupHeader.numerator = adspi.rupFramesNumerator;
		rupHeader.framesQuantity = static_cast<quint16>(adspi.rupFramesCount);

		QDateTime dt = extAppData.timeStamp.toDateTime();

		rupHeader.timeStamp.millisecond = static_cast<quint16>(dt.time().msec());
		rupHeader.timeStamp.second = static_cast<quint16>(dt.time().second());
		rupHeader.timeStamp.minute = static_cast<quint16>(dt.time().minute());
		rupHeader.timeStamp.hour = static_cast<quint16>(dt.time().hour());

		rupHeader.timeStamp.day = static_cast<quint16>(dt.date().day());
		rupHeader.timeStamp.month = static_cast<quint16>(dt.date().month());
		rupHeader.timeStamp.year = static_cast<quint16>(dt.date().year());

		rupHeader.reverseBytes();

		const int RUP_FRAME_DATA_SIZE = sizeof(rupFrame.data);

		for(int frameNo = 0; frameNo < adspi.rupFramesCount; frameNo++)
		{
			rupHeader.frameNumber = reverseUint16(static_cast<quint16>(frameNo));

			int inFrameDataSize = extAppData.appData.size() - (frameNo * RUP_FRAME_DATA_SIZE);

			if (inFrameDataSize > RUP_FRAME_DATA_SIZE)
			{
				inFrameDataSize = RUP_FRAME_DATA_SIZE;
			}

			memcpy(rupFrame.data, extAppData.appData.constData() + (frameNo * RUP_FRAME_DATA_SIZE), inFrameDataSize);

			if (inFrameDataSize < RUP_FRAME_DATA_SIZE)
			{
				memset(rupFrame.data + inFrameDataSize, 0, RUP_FRAME_DATA_SIZE - inFrameDataSize);
			}

			rupFrame.calcCRC64();

			simFrame.sourceIP = reverseUint32(adspi.lanSourceIP.toIPv4Address());

			m_socket->writeDatagram(reinterpret_cast<const char*>(&simFrame),
									sizeof(simFrame),
									adspi.lanDestinationIP,
									static_cast<quint16>(adspi.lanDestinationPort));
		}

		adspi.rupFramesNumerator++;
	}
}
