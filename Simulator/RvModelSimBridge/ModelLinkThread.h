#pragma once

#include <QUdpSocket>
#include <vector>
#include <queue>

#include "../../UtilsLib/SimpleThread.h"
#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../AppSignalLib/TuningDataStorage.h"
#include "ModelLinkPacket.h"


	// ----------------------------------------------------------------------------------
	//
	// UdpModelLink class declaration
	//
	// ----------------------------------------------------------------------------------

	class ModelSimBridgeWorker;

	class UdpModelLink : public SimpleThreadWorker
	{
	public:
		UdpModelLink(const HostAddressPort& listenIP, std::shared_ptr<CircularLogger> loggerr);
		~UdpModelLink();

	private:
		void onThreadStarted() override;
		void onThreadFinished() override;

		void timerEvent(QTimerEvent* event) override;

		void initTimer();
		void shutdownTimer();

		void createSocket();
		void closeSocket();
		bool readSocket();
		bool writeSocket();

		bool processModelPacket(const ModelLink::SimulatorBridgePacket& packet);

	private:
		HostAddressPort m_listenIP;

		std::shared_ptr<CircularLogger> m_logger;

		//

		QBasicTimer* m_timer = nullptr;
		QUdpSocket* m_socket = nullptr;
		qint64 m_socketCreateLastTime = 0;

		// statistics
		//
		qint64 m_errReplySize = 0;
		qint64 m_errReadSocket = 0;
		qint64 m_errUnknownTuningSource = 0;
		qint64 m_errSimVersion = 0;
		qint64 m_errNotExpectedSimPacket = 0;
	};

	class UdpModelLinkThread : public SimpleThread
	{
	public:
		UdpModelLinkThread(UdpModelLink* worker);
	};

