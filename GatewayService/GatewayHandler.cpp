#include "GatewayHandler.h"
#include "IvsImpulseGatewayHandler.h"
#include "ModbusTcpSlaveGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Gateway::Handler class implementation
	//
	// ---------------------------------------------------------------------------------

	Handler::Handler(const QString& gatewayID,
					 const SoftwareInfo& swInfo,
					 const GatewayServiceSettings& settings,
					 CircularLoggerShared log, bool logGatewayPackets) :
		m_gatewayID(gatewayID),
		m_swInfo(swInfo),
		m_settings(settings),
		m_log(log),
		m_logGatewayPackets(logGatewayPackets)
	{
		if (m_logGatewayPackets)
		{
			m_logStartTimeSecs = QDateTime::currentSecsSinceEpoch();
			m_gwLog = std::make_shared<CircularLogger>();

			m_gwLog->init(gatewayID, Separator::EMPTY_STR, 10, 20);
		}
	}

	Handler::~Handler()
	{
		Q_ASSERT(m_shutwownCalled);
	}

	void Handler::shutdown()
	{
		m_shutwownCalled = true;
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

	CircularLoggerShared Handler::log()
	{
		return m_log;
	}

	bool Handler::enableLogging() const
	{
		return m_logGatewayPackets;
	}

	void Handler::logRequest(const QString& msg, CircularLogger::RecordType recType)
	{
		if (m_logGatewayPackets == false ||
			m_gwLog == nullptr)
		{
			return;
		}

		if (m_lastMsgIsRequest == false)
		{
			LOG_MSG(m_log, Separator::EMPTY_STR);
		}

		m_lastMsgIsRequest = true;

		QString logMsg = QStringLiteral("Request: ");

		logMsg.append(msg);

		writeToGwLog(logMsg, recType);
	}

	void Handler::logReply(const QString& msg, CircularLogger::RecordType recType)
	{
		if (m_logGatewayPackets == false ||
			m_gwLog == nullptr)
		{
			return;
		}

		if (m_lastMsgIsRequest == true)
		{
			LOG_MSG(m_log, Separator::EMPTY_STR);
		}

		m_lastMsgIsRequest = false;

		QString logMsg = QStringLiteral("Replay:  ");

		logMsg.append(msg);

		writeToGwLog(logMsg, recType);
	}

	void Handler::writeToGwLog(const QString& msg, CircularLogger::RecordType recType)
	{
		if (m_logGatewayPackets == false)
		{
			return;
		}

		if (m_gwLog == nullptr ||
			m_logStartTimeSecs == 0)
		{
			return;
		}

		qint64 curTimeSecs = QDateTime::currentSecsSinceEpoch();

		if (curTimeSecs - m_logStartTimeSecs > GW_LOG_PERIOD_SECS)
		{
			m_logGatewayPackets = false;
			m_logStartTimeSecs = 0;
			m_gwLog->shutdown();
			m_gwLog.reset();
		}

		switch(recType)
		{
		case CircularLogger::RecordType::Error:
			LOG_ERR(m_gwLog, msg);
			break;

		case CircularLogger::RecordType::Warning:
			LOG_WRN(m_gwLog, msg);
			break;

		case CircularLogger::RecordType::Message:
			LOG_MSG(m_gwLog, msg);
			break;

		case CircularLogger::RecordType::Config:
		default:
			Q_ASSERT(false);
		}
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
						QString logGatewayIDs)
	{
		Q_ASSERT(m_handlers.empty());

		logGatewayIDs.replace(Separator::COMMA, Separator::SPACE);

		QStringList logGwIDs = logGatewayIDs.split(Separator::SPACE, Qt::SkipEmptyParts);

		bool result = true;

		for(const GatewayShared& gw : gateways)
		{
			if (gw->enable() == false)
			{
				DEBUG_LOG_WRN(log, QString("Gateway %1 disabled so NOT RUN!"));
				continue;
			}

			bool enableLogging = logGwIDs.contains(gw->gatewayID());

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
																log, enableLogging);

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
															log, enableLogging);

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
