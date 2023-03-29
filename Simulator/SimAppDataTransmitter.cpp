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
		stopTransmitterThread();
	}

	bool AppDataTransmitter::startSimulation(QString profileName)
	{
		// m_log.writeText(QString("Sending app data simulation is started for profile %1").arg(profileName));

		TEST_PTR_RETURN_FALSE(m_simulator);
		m_curProfileName = profileName;

		initAppDataSources();
		runTransmitterThread();

		return true;
	}

	bool AppDataTransmitter::stopSimulation()
	{
		stopTransmitterThread();
		m_appDataSourcePorts.clear();

		return true;
	}

	bool AppDataTransmitter::sendData(const QString& lmEquipmentId,
									  const QString& portEquipmentId,
									  const QByteArray& data,
									  TimeStamp timeStamp)
	{
		m_appDataQueueMutex.lock();

		m_appDataQueue.emplace(lmEquipmentId,
							  portEquipmentId,
							  data,
							  timeStamp);

		m_appDataQueueNotEmpty.notify_one();

		m_appDataQueueMutex.unlock();

		return true;
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

	void AppDataTransmitter::initAppDataSources()
	{
		TEST_PTR_RETURN(m_simulator);

		std::vector<std::shared_ptr<LogicModule>> logicModules = m_simulator->logicModules();

		m_appDataSourcePorts.clear();

		for(std::shared_ptr<LogicModule> logicModule : logicModules)
		{
			TEST_PTR_CONTINUE(logicModule);

			::LogicModuleInfo lmi = logicModule->logicModuleExtraInfo();

			if (lmi.appDataEnable == false)
			{
				continue;
			}

			for(const LanControllerInfo& lci : lmi.lanControllers())
			{
				if (lci.isAppDataEnabled() == true)
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
							m_simulator->software().getSettingsProfile<AppDataServiceSettings>(lci.appDataServiceID, m_curProfileName);

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

	void AppDataTransmitter::runTransmitterThread()
	{
		Q_ASSERT(m_transmitterThread == nullptr);

		m_runSimulation = true;
		m_transmitterThreadWork = false;

		m_transmitterThread = new std::thread(&AppDataTransmitter::processAppDataQueue, this);
	}

	void AppDataTransmitter::stopTransmitterThread()
	{
		if (m_transmitterThread != nullptr)
		{
			//

			m_runSimulation = false;

			while(m_transmitterThreadWork == true)
			{
				m_appDataQueueMutex.lock();
				m_appDataQueueNotEmpty.notify_one();
				m_appDataQueueMutex.unlock();
			}

			//

			m_transmitterThread->join();

			delete m_transmitterThread;
			m_transmitterThread = nullptr;
		}
	}

	void AppDataTransmitter::processAppDataQueue()
	{
		m_transmitterThreadWork = true;

		qDebug() << "processAppDataQueue started";

		QUdpSocket socket;
		ExtAppData extAppData;

		std::unique_lock ul(m_appDataQueueMutex, std::defer_lock);

		int maxQueueSize = 0;

		while(m_runSimulation == true)
		{
			ul.lock();

			m_appDataQueueNotEmpty.wait(ul);

			// ul locked here

			while(true)
			{
				if (m_appDataQueue.empty() == true ||
					m_runSimulation == false)
				{
					ul.unlock();
					break;
				}

				maxQueueSize = std::max(static_cast<int>(m_appDataQueue.size()), maxQueueSize);

				ExtAppData& queueAppData = m_appDataQueue.front();

				extAppData.lmEquipmentID = queueAppData.lmEquipmentID;
				extAppData.portEquipmentID = queueAppData.portEquipmentID;
				extAppData.timeStamp = queueAppData.timeStamp;
				extAppData.appData.swap(queueAppData.appData);

				m_appDataQueue.pop();

				ul.unlock();

				sendAppDataPackets(socket, extAppData);

				ul.lock();
			}
		}

		qDebug() << "processAppDataQueue finished maxSize =" << maxQueueSize;

		m_transmitterThreadWork = false;
	}

	void AppDataTransmitter::sendAppDataPackets(QUdpSocket& socket, const ExtAppData& extAppData)
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

		rupHeader.timeStamp.setDateTime(extAppData.timeStamp.toDateTime());

		rupHeader.reverseBytes();

		const int RUP_FRAME_DATA_SIZE = sizeof(rupFrame.data);

		for(int frameNo = 0; frameNo < adspi.rupFramesCount; frameNo++)
		{
			rupHeader.frameNumber = reverseUint16(static_cast<quint16>(frameNo));

			int inFrameDataSize = static_cast<int>(extAppData.appData.size()) - (frameNo * RUP_FRAME_DATA_SIZE);

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

			socket.writeDatagram(reinterpret_cast<const char*>(&simFrame),
									sizeof(simFrame),
									adspi.lanDestinationIP,
									static_cast<quint16>(adspi.lanDestinationPort));
		}

		adspi.rupFramesNumerator++;
	}
}
