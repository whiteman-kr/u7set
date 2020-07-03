#include "SimAppDataTransmitter.h"
#include "Simulator.h"

#include "../lib/LogicModulesInfo.h"
#include "../lib/DataProtocols.h"

namespace Sim
{

	AppDataTransmitter::AppDataTransmitter(Simulator* simulator) :
		Output("AppDataTransmitter"),
		m_simulator(simulator)
	{
		Q_ASSERT(m_simulator);

		connect(m_simulator, &Simulator::projectUpdated, this, &AppDataTransmitter::projectUpdated);
	}

	AppDataTransmitter::~AppDataTransmitter()
	{
		shutdownTransmitterThread();
	}

	bool AppDataTransmitter::startSimulation()
	{
		TEST_PTR_RETURN_FALSE(m_simulator);

		m_transmitterThread = new AppDataTransmitterThread(*m_simulator);
		m_transmitterThread->start();

		return true;
	}

	bool AppDataTransmitter::stopSimulation()
	{
		shutdownTransmitterThread();

		return true;
	}

	bool AppDataTransmitter::sendData(const QString& lmEquipmentId, QByteArray&& data, TimeStamp timeStamp)
	{
		TEST_PTR_RETURN_FALSE(m_transmitterThread);

		return m_transmitterThread->sendAppData(lmEquipmentId, data, timeStamp);
	}

	void AppDataTransmitter::projectUpdated()
	{
		// Project was loaded or cleared
		// Reset all queues here
		//
	}

	bool AppDataTransmitter::enabled() const
	{
		return m_enabled;
	}

	void AppDataTransmitter::setEnabled(bool value)
	{
		m_enabled = value;
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

	AppDataTransmitterThread::AppDataTransmitterThread(const Simulator& simulator) :
		m_simulator(simulator)
	{
	}

	AppDataTransmitterThread::~AppDataTransmitterThread()
	{
	}

	bool AppDataTransmitterThread::sendAppData(const QString& lmEquipmentId, QByteArray& data, TimeStamp timeStamp)
	{
		QThread* curThread = QThread::currentThread();

		m_appDataQueueMutex.lock(curThread);

		ExtAppData extAppData;

		extAppData.lmEquipmentID = lmEquipmentId;
		extAppData.timeStamp = timeStamp;

		m_appDataQueue.push(extAppData);

		m_appDataQueue.back().appData.swap(data);			// !

		m_appDataQueueMutex.unlock(curThread);

		return true;
	}

	void AppDataTransmitterThread::run()
	{
		initAppDataSources();

		m_socket = new QUdpSocket(this);

		QThread* curThread = QThread::currentThread();

		ExtAppData extAppData;

		while(isQuitRequested() == false)
		{
			m_appDataQueueMutex.lock(curThread);

			if (m_appDataQueue.empty() == false)
			{
				ExtAppData& queueAppData = m_appDataQueue.front();

				extAppData.lmEquipmentID = queueAppData.lmEquipmentID;
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

			AppDataSourceInfo adsi;

			adsi.appDataUID = lmi.appDataUID;
			adsi.appDataSizeBytes = lmi.appDataSizeBytes;

			adsi.moduleType = lmi.moduleType();

			adsi.rupFramesCount = (adsi.appDataSizeBytes / sizeof(Rup::Data)) +
									((adsi.appDataSizeBytes % sizeof(Rup::Data)) == 0 ? 0 : 1);

			for(LanControllerInfo lci : lmi.lanControllers)
			{
				if (lci.appDataProvided == true && lci.appDataEnable == true)
				{
					LanController lanController;

					lanController.sourceIP = QHostAddress(lci.appDataIP);
					lanController.sourcePort = lci.appDataPort;

					lanController.destinationIP = QHostAddress(lci.appDataServiceIP);
					lanController.destinationPort = lci.appDataServicePort;

					adsi.lanControllers.push_back(lanController);
				}
			}

			m_appDataSources.insert({lmi.equipmentID, adsi});
		}
	}

	void AppDataTransmitterThread::privateSendAppData(const ExtAppData& extAppData)
	{
		auto item = m_appDataSources.find(extAppData.lmEquipmentID);

		if (item == m_appDataSources.end())
		{
			Q_ASSERT(false);
			return;
		}

		AppDataSourceInfo& adsi = item->second;

		Q_ASSERT(extAppData.appData.size() == adsi.appDataSizeBytes);

		Rup::SimFrame simFrame;

		simFrame.simVersion = reverseUint16(1);

		Rup::Frame& rupFrame = simFrame.rupFrame;

		Rup::Header& rupHeader = rupFrame.header;

		rupHeader.frameSize = sizeof(rupFrame);
		rupHeader.protocolVersion = 5;

		rupHeader.flags.all = 0;
		rupHeader.flags.appData = 1;

		rupHeader.dataId = adsi.appDataUID;
		rupHeader.moduleType = static_cast<quint16>(adsi.moduleType);
		rupHeader.numerator = adsi.rupFramesNumerator;
		rupHeader.framesQuantity = adsi.rupFramesCount;

		QDateTime dt = extAppData.timeStamp.toDateTime();

		rupHeader.timeStamp.millisecond = dt.time().msec();
		rupHeader.timeStamp.second = dt.time().second();
		rupHeader.timeStamp.minute = dt.time().minute();
		rupHeader.timeStamp.hour = dt.time().hour();

		rupHeader.timeStamp.day = dt.date().day();
		rupHeader.timeStamp.month = dt.date().month();
		rupHeader.timeStamp.year = dt.date().year();

		rupHeader.reverseBytes();

		const int RUP_FRAME_DATA_SIZE = sizeof(rupFrame.data);

		for(int frameNo = 0; frameNo < adsi.rupFramesCount; frameNo++)
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

			for(LanController& lanController: adsi.lanControllers)
			{
				simFrame.sourceIP = reverseUint32(lanController.sourceIP.toIPv4Address());

				m_socket->writeDatagram(reinterpret_cast<const char*>(&simFrame),
										sizeof(simFrame),
										lanController.destinationIP,
										lanController.destinationPort);
			}
		}

		adsi.rupFramesNumerator++;
	}
}
