#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "GatewayDescription.h"
#include "AppSignalState.h"
#include "IvsImpulseDataProtocol.h"

namespace Gateway
{
	struct IvsImpulseListInfo;
	class IvsImpulseHandler;

	class IvsImpulseCommThreadWorker : public SimpleThreadWorker
	{
	public:
		IvsImpulseCommThreadWorker(IvsImpulseHandler& handler);

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void onTimer();

		void periodicSendStates();
		int writeStatesToPacket(IvsImpulseStatesPacket* packet,
								E::SignalListDataType dataType,
								int startIndex, int size, int& paramCount);

		int writeStatesToPacket_A(AnalogState_A* states,
								int startIndex, int size, int& paramCount);

		int writeStatesToPacket_B(DiscreteState_B* states,
								int startIndex, int size, int& paramCount);

		int writeStatesToPacket_D(DiscreteState_D* states,
								int startIndex, int size, int& paramCount);

		AnalogStateCode_A getAnalogStateCodeA(::AppSignalStateFlags flags) const;

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
		std::vector<IvsImpulseListInfo>& m_lists;

		std::vector<GatewayChannelInfo> m_channelsInfo;

		char m_sendBuffer[IVS_IMPULSE_PACKET_MAX_SIZE + 100];

		//

		QTimer m_timer;
		QUdpSocket m_socket;
	};

	class IvsImpulseCommThread : public SimpleThread
	{
	public:
		IvsImpulseCommThread(IvsImpulseHandler& handler)
		{
			addWorker(new IvsImpulseCommThreadWorker(handler));
		}
	};
}
