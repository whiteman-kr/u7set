#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "GatewayDescription.h"
#include "AppSignalState.h"
#include "IvsImpulseDataProtocol.h"

namespace Gateway
{
	class IvsImpulseListInfo;
	class IvsImpulseHandler;
	class AppDataServiceClient;

	class IvsImpulseCommThreadWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		IvsImpulseCommThreadWorker(IvsImpulseHandler& handler);

	public slots:
		void onSendStateChanges();

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void onTimer();

		bool tryCreateSockets();
		bool isWorkableSocketExists() const;
		void periodicSendStates();
		void sendStateChanges();

		void sendPacket(const char* packet, qint64 packetSize, bool eventsPacket);

		void logEventsPacket(const char* packet);
		void logPeriodicPacket(const char* packet);

		void checkLogTime();

		qint64 convertTimeToUTC(quint64 time, ::E::TimeType timeType) const;

		QString formatTime(quint32 seconds);

		int writeStatesToPacket(IvsImpulseStatesPacket* packet,
								E::SignalListDataType dataType,
								int startIndex, int size,
								int& paramCount, qint64& time);

		int writeStatesToPacket_A(AnalogState_A* states,
								  int startIndex, int size,
								  int& paramCount, qint64& time);

		int writeStatesToPacket_B(DiscreteState_B* states,
								  int startIndex, int size,
								  int& paramCount, qint64& time);

		int writeStatesToPacket_D(DiscreteState_D* states,
								  int startIndex, int size,
								  int& paramCount, qint64& time);

		int writeStateChangesToPacket(std::shared_ptr<IvsImpulseListInfo> &li,
									  IvsImpulseSignalEvent* events,
									  E::SignalListDataType dataType,
									  qint64& baseTime_ms,
									  const std::vector<GatewayAppSignalState>& stateChanges,
									  int& paramCount);

		AnalogStateCode_A getAnalogStateCodeA(const SimpleAppSignalState& state) const;
		DiscreteState_D getDiscreteStateD(const SimpleAppSignalState& state) const;

	private:

		static const int TRY_CREATE_SOCKET_INTERVAL_MS = 3000;

		struct GatewayChannelInfo
		{
			GatewayChannelInfo(const HostAddressPort& localIP, const HostAddressPort& remoteIP)
			{
				 localGatewayIP = localIP;
				 remoteGatewayIP = remoteIP;
			}

			~GatewayChannelInfo()
			{
				clearSocket();
			}

			void clearSocket()
			{
				DELETE_IF_NOT_NULL(socket);

				prevTryCreateSocketTime = QDateTime::currentMSecsSinceEpoch();
			}

			HostAddressPort localGatewayIP;
			HostAddressPort remoteGatewayIP;

			//

			bool tryCreateSocket(CircularLoggerShared log);

			qint64 prevTryCreateSocketTime = 0;
			QUdpSocket* socket = nullptr;

			int statesPacketsSentCount = 0;
			int eventPacketsSentCount = 0;
		};


	private:
		CircularLoggerShared m_log;
		IvsImpulseGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		const AppSignalStates& m_states;
		std::atomic_bool& m_signalStatesUpdated;

		std::vector<std::shared_ptr<IvsImpulseListInfo>>& m_lists;

		std::vector<GatewayChannelInfo> m_channelsInfo;

		std::vector<qint64> m_eventsTimes;

		char m_sendBuffer[IVS_IMPULSE_PACKET_MAX_SIZE + 100];

		bool m_logGatewayPackets = false;
		CircularLoggerShared m_packetsLog;
		qint64 m_logStartTime = 0;
		int m_checkLogTimeCtr = 0;

		//

		QTimer m_timer;
	};

	class IvsImpulseCommThread : public SimpleThread
	{
	public:
		IvsImpulseCommThread(IvsImpulseHandler& handler);
		void connect(AppDataServiceClient* client);
	};
}
