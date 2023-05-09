#pragma once

#include "../UtilsLib/SimpleThread.h"
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

		void periodicSendStates();
		void sendStateChanges();

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
									  qint64 baseTime_ms,
									  const std::vector<GatewayAppSignalState>& stateChanges,
									  int& paramCount);

		AnalogStateCode_A getAnalogStateCodeA(const SimpleAppSignalState& state) const;
		DiscreteState_D getDiscreteStateD(const SimpleAppSignalState& state) const;

	private:
		struct GatewayChannelInfo
		{
			GatewayChannelInfo(const HostAddressPort& ip)
			{
				 gatewayIP = ip;
			}

			HostAddressPort gatewayIP;

			int statesPacketsSentCount = 0;
			int eventPacketsSentCount = 0;
		};


	private:
		IvsImpulseGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		const AppSignalStates& m_states;
		std::atomic_bool& m_signalStatesUpdated;

		std::vector<std::shared_ptr<IvsImpulseListInfo>>& m_lists;

		std::vector<GatewayChannelInfo> m_channelsInfo;

		char m_sendBuffer[IVS_IMPULSE_PACKET_MAX_SIZE + 100];

		//

		QTimer m_timer;
		QUdpSocket m_socket;
	};

	class IvsImpulseCommThread : public SimpleThread
	{
	public:
		IvsImpulseCommThread(IvsImpulseHandler& handler);
		void connect(AppDataServiceClient* client);
	};
}
