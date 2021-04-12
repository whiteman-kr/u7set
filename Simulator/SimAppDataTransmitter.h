#pragma once

#include <queue>
#include <QUdpSocket>
#include "../lib/TimeStamp.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/SimpleMutex.h"
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
		bool startSimulation(QString profileName);
		bool stopSimulation();
		bool sendData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp);

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool softwareEnabled() const;			// Global enable for all LogicModules AppData LANs

	private:
		void shutdownTransmitterThread();

	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		AppDataTransmitterThread* m_transmitterThread = nullptr;
	};

	class AppDataTransmitterThread : public RunOverrideThread
	{
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
		AppDataTransmitterThread(const Simulator& simulator, const QString& curProfileName, ScopedLog& log);
		virtual ~AppDataTransmitterThread();

		bool sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp);

		virtual void run();

	private:
		void initAppDataSources();
		void privateSendAppData(const ExtAppData& extAppData);

	private:
		const Simulator& m_simulator;
		const QString m_curProfileName;
		ScopedLog m_log;

		mutable SimpleMutex m_appDataQueueMutex;
		std::queue<ExtAppData> m_appDataQueue;

		std::unordered_map<QString, AppDataSourcePortInfo> m_appDataSourcePorts;

		QUdpSocket* m_socket = nullptr;
	};
}




