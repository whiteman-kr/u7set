#include "IvsImpulseCommThread.h"
#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	// --------------------------------------------------------------------------------------
	//
	//  IvsImpulseCommThreadWorker class implementation
	//
	// --------------------------------------------------------------------------------------

	IvsImpulseCommThreadWorker::IvsImpulseCommThreadWorker(IvsImpulseHandler& handler) :
		m_gateway(handler.m_gateway),
		m_appSignals(handler.m_appSignals),
		m_states(handler.m_states),
		m_lists(handler.m_lists),
		m_gatewayIP1(handler.m_gateway->gatewayIP1()),
		m_gatewayIP2(handler.m_gateway->gatewayIP2()),
		m_timer(this),
		m_socket(this)
	{
	}

	void IvsImpulseCommThreadWorker::onThreadStarted()
	{
		m_timer.setTimerType(Qt::PreciseTimer);
		m_timer.setInterval(m_gateway->period());
		m_timer.setSingleShot(false);

		connect(&m_timer, &QTimer::timeout, this, &IvsImpulseCommThreadWorker::onTimer);

		m_timer.start();
	}

	void IvsImpulseCommThreadWorker::onThreadFinished()
	{
		m_timer.stop();
	}

	void IvsImpulseCommThreadWorker::onTimer()
	{
		for(const IvsImpulseListInfo& li : m_lists)
		{
			periodicSendStates(li);
		}
	}

	void IvsImpulseCommThreadWorker::periodicSendStates(const IvsImpulseListInfo& listInfo)
	{
		char buffer[2048];
		int len = 2048;

		// prepare buffer

		if (m_gatewayIP1.isSet())
		{
			m_socket.writeDatagram(buffer, len, m_gatewayIP1.address(), m_gatewayIP1.port());
		}

		if (m_gatewayIP2.isSet())
		{
			m_socket.writeDatagram(buffer, len, m_gatewayIP2.address(), m_gatewayIP2.port());
		}
	}
}

