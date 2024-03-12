#include "GatewayHandler.h"
#include "IvsImpulseGatewayHandler.h"
#include "ModbusTcpSlaveGatewayHandler.h"
#include "../ServiceLib/Service.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Gateway::Handler class implementation
	//
	// ---------------------------------------------------------------------------------

	Handler::Handler(const SoftwareInfo& swInfo,
					 const GatewayServiceSettings& settings,
					 CircularLoggerShared log, bool logGatewayPackets) :
		m_swInfo(swInfo),
		m_settings(settings),
		m_log(log),
		m_logGatewayPackets(logGatewayPackets)
	{
	}

	void Handler::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		Q_UNUSED(hashes);
	}

	void Handler::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		Q_UNUSED(hashes);
	}

	void Handler::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
	{
		Q_UNUSED(getStatesReply);
	}

	void Handler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
		Q_UNUSED(getStateChangesReply);
	}

	// ---------------------------------------------------------------------------------
	//
	// Gateway::Handlers class implementation
	//
	// ---------------------------------------------------------------------------------

	Handlers::Handlers()
	{
	}

	bool Handlers::init(const Gateways& gateways,
						const SoftwareInfo& swInfo,
						const GatewayServiceSettings& settings,
						const AppSignals& appSignals,
						CircularLoggerShared log,
						bool logGatewayPackets)
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

					IvsImpulseHandlerShared ivsHandler =
							std::make_shared<IvsImpulseHandler>(swInfo, settings, ivsGateway, appSignals,
																log, logGatewayPackets);

					m_handlers.push_back(ivsHandler);
				}
				break;

			case E::GatewayType::ModbusTcpSlave:
				{
					ModbusTcpSlaveGatewayShared modbusGateway = std::dynamic_pointer_cast<ModbusTcpSlaveGateway>(gw);

					if (modbusGateway == nullptr)
					{
						result = false;
						break;
					}

					ModbusTcpSlaveHandlerShared modbusHandler =
						std::make_shared<ModbusTcpSlaveHandler>(swInfo, settings, modbusGateway, appSignals,
															log, logGatewayPackets);

					m_handlers.push_back(modbusHandler);
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
