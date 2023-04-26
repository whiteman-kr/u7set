#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "GatewayDescription.h"
#include "AppSignalState.h"

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

		void periodicSendStates(const IvsImpulseListInfo& listInfo);

	private:
		IvsImpulseGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		const AppSignalStates& m_states;
		const std::vector<IvsImpulseListInfo>& m_lists;

		HostAddressPort m_gatewayIP1;
		HostAddressPort m_gatewayIP2;

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
