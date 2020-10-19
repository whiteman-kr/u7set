#pragma once

#include <queue>
#include <QUdpSocket>
#include "../lib/TimeStamp.h"
#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"
#include "../lib/WUtils.h"
#include "../lib/SimpleMutex.h"
#include "../lib/ILogFile.h"
#include "SimScopedLog.h"

namespace Sim
{
	class Simulator;
	class AppDataTransmitterThread;

	class AppDataTransmitter : public QObject
	{
		Q_OBJECT

	public:
		AppDataTransmitter(Simulator* simulator);
		virtual ~AppDataTransmitter();

	public:
		bool startSimulation();
		bool stopSimulation();
		bool sendData(const QString& lmEquipmentId, QByteArray&& data, TimeStamp timeStamp);

	protected slots:
		void projectUpdated();		// Project was loaded or cleared

	public:
		bool enabled() const;
		void setEnabled(bool value);

	private:
		void shutdownTransmitterThread();

	private:
		Simulator* m_simulator;
		mutable ScopedLog m_log;
		std::atomic<bool> m_enabled{false};			// Allow AppData trasmittion to AppDataSrv

		AppDataTransmitterThread* m_transmitterThread = nullptr;
	};

	//

	class AppDataTransmitterThread : public RunOverrideThread
	{
	private:
		struct ExtAppData
		{
			QString lmEquipmentID;
			QByteArray appData;
			TimeStamp timeStamp;
		};

		struct LanController
		{
			QHostAddress sourceIP;
			int sourcePort = 0;

			QHostAddress destinationIP;
			int destinationPort = 0;
		};

		struct AppDataSourceInfo
		{
			quint32 appDataUID = 0;
			int appDataSizeBytes = 0;
			int moduleType = 0;

			std::vector<LanController> lanControllers;

			quint16 rupFramesNumerator = 0;
			int rupFramesCount = 0;
		};

	public:
		AppDataTransmitterThread(const Simulator& simulator);
		virtual ~AppDataTransmitterThread();

		bool sendAppData(const QString& lmEquipmentId, QByteArray& data, TimeStamp timeStamp);

		virtual void run();

	private:
		void initAppDataSources();
		void privateSendAppData(const ExtAppData& extAppData);

	private:
		const Simulator& m_simulator;

		mutable SimpleMutex m_appDataQueueMutex;
		std::queue<ExtAppData> m_appDataQueue;

		std::unordered_map<QString, AppDataSourceInfo> m_appDataSources;

		QUdpSocket* m_socket = nullptr;
	};
}




