#include "GatewayHandler.h"
#include "IvsImpulseGatewayHandler.h"
#include "ModbusSlaveGatewayHandler.h"
#include "AdsGatewayHandler.h"

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
			m_gwLog->setLogCodeInfo(false);
		}
	}

	Handler::~Handler()
	{
		Q_ASSERT(m_shutwownCalled);
	}

	void Handler::shutdown()
	{
		closeGwLog();

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

	QString Handler::gatewayID() const
	{
		return m_gatewayID;
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
			LOG_MSG(m_gwLog, Separator::EMPTY_STR);
		}

		m_lastMsgIsRequest = true;

		QString logMsg = QStringLiteral("=> ");

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
			LOG_MSG(m_gwLog, Separator::EMPTY_STR);
		}

		m_lastMsgIsRequest = false;

		QString logMsg = QStringLiteral("<= ");

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
			closeGwLog();
			return;
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

	void Handler::closeGwLog()
	{
		m_logGatewayPackets = false;
		m_logStartTimeSecs = 0;

		if (m_gwLog != nullptr)
		{
			m_gwLog->shutdown();
			m_gwLog.reset();
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
				DEBUG_LOG_WRN(log, QString("Gateway %1 disabled so NOT RUN!").arg(gw->gatewayID()));
				continue;
			}

			bool enableLogging = logGwIDs.contains(gw->gatewayID());

			if (enableLogging)
			{
				DEBUG_LOG_MSG(log, QString("1 hour request/reply detail logging turned ON for gateway %1").arg(gw->gatewayID()));
			}

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

			case E::GatewayType::ModbusSlave:
				{
					ModbusSlaveGatewayShared modbusGateway = std::dynamic_pointer_cast<ModbusSlaveGateway>(gw);

					if (modbusGateway == nullptr)
					{
						result = false;
						break;
					}

					ModbusSlaveHandlerShared modbusHandler =
						std::make_shared<ModbusSlaveHandler>(swInfo, settings, modbusGateway, appSignals,
															log, enableLogging);

					m_handlers.push_back(modbusHandler);
				}
				break;

			case E::GatewayType::AdsGateway:
			{
				AdsGatewayShared adsGateway = std::dynamic_pointer_cast<AdsGateway>(gw);

				if (adsGateway == nullptr)
				{
					result = false;
					break;
				}

				AdsGatewayHandlerShared adsGatewayHandler =
					std::make_shared<AdsGatewayHandler>(swInfo, settings, adsGateway, appSignals,
														 log, enableLogging);

				m_handlers.push_back(adsGatewayHandler);
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
