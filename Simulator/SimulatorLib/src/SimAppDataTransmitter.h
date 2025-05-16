#pragma once

#include <CommonLib/Times.h>
#include "SimScopedLog.h"


namespace Sim
{
	class SimulatorPrivate;

	class AppDataTransmitter : public QObject
	{
		Q_OBJECT

	private:
		struct ExtAppData
		{
			QString lmEquipmentID;
			QString portEquipmentID;
			QByteArray appData;
			TimeStamp timeStamp;
		};

		struct AppDataSourcePortInfo
		{
			QString equipmentID;

			quint32 appDataUID = 0;
			int appDataSizeBytes = 0;
			int moduleType = 0;

			//

			QHostAddress lanSourceIP;
			int lanSourcePort = 0;

			QHostAddress lanDestinationIP;
			int lanDestinationPort = 0;

			//

			quint16 rupFramesNumerator = 0;
			int rupFramesCount = 0;
		};

	public:
		AppDataTransmitter(SimulatorPrivate* simulator);
		virtual ~AppDataTransmitter();

	public:
		bool startSimulation(QString profileName);
		bool stopSimulation();
		bool sendData(const QString& lmEquipmentId,
					  const QString& portEquipmentId,
					  const QByteArray& data,
					  TimeStamp timeStamp);

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool softwareEnabled() const;			// Global enable for all LogicModules AppData LANs

	private:
		void initAppDataSources();

		void runTransmitterThread();
		void stopTransmitterThread();
		void wakeupTransmitterThread();

		void processAppDataQueue();
		void sendAppDataPackets(QUdpSocket& socket, const ExtAppData& extAppData);
		void clearAppDataQueue();

		void logTime(const QString& msg);
		void trace_dt(const QString& portID);

	private:
		SimulatorPrivate* m_simulator = nullptr;
		QString m_curProfileName;
		mutable ScopedLog m_log;

		//

		std::unordered_map<QString, AppDataSourcePortInfo> m_appDataSourcePorts;

		//

		mutable std::mutex m_appDataQueueMutex;
		std::condition_variable m_appDataQueueNotEmpty;
		std::queue<ExtAppData> m_appDataQueue;

		//

		qint64 m_simStartTime = 0;
		qint64 m_prevPacketTime = 0;

		std::atomic<bool> m_runSimulation = {false};
		std::atomic<bool> m_exitTransmitterThread = {false};
		std::thread* m_transmitterThread = nullptr;
	};
}




