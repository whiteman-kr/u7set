#include "GatewayHandler.h"
#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Handler implementation
	//
	// ---------------------------------------------------------------------------------

	Handler::Handler()
	{
	}

	// ---------------------------------------------------------------------------------
	//
	// Class GatewayHandlers implementation
	//
	// ---------------------------------------------------------------------------------

	Handlers::Handlers()
	{
	}

	bool Handlers::init(const Gateways& gateways, const AppSignals& appSignals)
	{
		Q_ASSERT(m_handlers.empty());

		bool result = true;

		for(const GatewayShared& gw : gateways)
		{
			switch(gw->gatewayType())
			{
			case E::GatewayType::IVS_Impulse:
				{
					IvsImpulseGatewayShared ivsGateway = std::dynamic_pointer_cast<IvsImpulseGateway>(gw);

					if (ivsGateway == nullptr)
					{
						result = false;
						break;
					}

					IvsImpulseHandlerShared ivsHandler = std::make_shared<IvsImpulseHandler>();

					result &= ivsHandler->init(ivsGateway, appSignals);

					BREAK_IF_FALSE(result);

					m_handlers.push_back(ivsHandler);
				}
				break;

			default:
				Q_ASSERT(false);
				result = false;
			}

			if (result == false)
			{
				m_handlers.clear();
				break;
			}
		}

		return result;
	}

	void Handlers::run()
	{
		for(HandlerShared& h : m_handlers)
		{
			h->run();
		}
	}

	void Handlers::shutdown()
	{
		for(HandlerShared& h : m_handlers)
		{
			h->shutdown();
		}

		m_handlers.clear();
	}

	void Handlers::clear()
	{
		m_handlers.clear();
	}
}
