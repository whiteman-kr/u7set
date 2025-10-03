#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TcpSignalClient.h"
#include "../AppSignalLib/DiscretesLogRecord.h"

#include <ClientLib/SignalLog.h>


namespace
{
	thread_local ::Network::GetSignalListStartReply tl_getSignalListStartReply;

	thread_local ::Network::GetSignalListNextRequest tl_getSignalListNextRequest;
	thread_local ::Network::GetSignalListNextReply tl_getSignalListNextReply;

	thread_local ::Network::GetAppSignalParamRequest tl_getSignalParamRequest;
	thread_local ::Network::GetAppSignalParamReply tl_getSignalParamReply;

	thread_local ::Network::GetAppSignalStateChangesRequest tl_getSignalStateChangesRequest;
	thread_local ::Network::GetAppSignalStateChangesReply tl_getSignalStateChangesReply;

	thread_local ::Network::GetAppSignalStateRequest tl_getSignalStateRequest;
	thread_local ::Network::GetAppSignalStateReply tl_getSignalStateReply;

	thread_local ::Network::GetDiscretesLogReply tl_getDiscretesLogReply;
} // namespace

namespace ClientLib
{
	TcpSignalClient::TcpSignalClient(const SoftwareInfo& softwareInfo,
									 const SoftwareEndpoint::AppDataService& adsInfo,
									 IAppSignalUpdater& signalUpdater,
									 SignalLog& signalLog,
									 ILogFile* logFile) :
		Tcp::Client(softwareInfo, adsInfo.address, "TcpSignalClient", adsInfo.equipmentId),
		TcpClientStatistics(this),
		HasLogFile(logFile, QString("ADS ") + adsInfo.shortenId),
		m_serverSettings(adsInfo),
		m_signalUpdater(signalUpdater),
		m_signalLog(signalLog)
	{
		setObjectName("TcpSignalClient " + adsInfo.equipmentId);

		Q_ASSERT(this->logFile());
		writeMessage(
			QString("TcpSignalClient::TcpSignalClient() %1, %2").arg(adsInfo.equipmentId).arg(serverAddressPort1().addressPortStr()));

		auto logger = std::make_shared<CircularLogger>(logFile, QString("TSC %1").arg(adsInfo.shortenId));
		setLogger(logger);

		connect(this,
				&Tcp::Client::signal_wrongServerID,
				[this](const QString& errorMessage)
				{
					writeError(errorMessage);
				});

		return;
	}

	TcpSignalClient::~TcpSignalClient()
	{
		writeMessage(QString("TcpSignalClient::~TcpSignalClient() %1").arg(serverAddressPort1().addressPortStr()));
	}

	void TcpSignalClient::onClientThreadStarted()
	{
		writeMessage(QString("TcpSignalClient::onClientThreadStarted() %1").arg(serverAddressPort1().addressPortStr()));
		return;
	}

	void TcpSignalClient::onClientThreadFinished()
	{
		writeMessage(QString("TcpSignalClient::onClientThreadFinished() %1").arg(serverAddressPort1().addressPortStr()));

		// theSignals.reset();	!!!signal reset moved to AdsConnection::configurationArrived
		return;
	}

	void TcpSignalClient::onConnection()
	{
		writeMessage(QString("TcpSignalClient::onConnection() %1").arg(serverAddressPort1().addressPortStr()));

		Q_ASSERT(isClearToSendRequest() == true);

		// Reset the date when the time discrepancy was checked.
		//
		m_timeDiscrepancyCheckDate = {};

		resetToGetSignalList();

		return;
	}

	void TcpSignalClient::onDisconnection()
	{
		writeMessage(QString("TcpSignalClient::onDisconnection() %1").arg(serverAddressPort1().addressPortStr()));

		m_signalUpdater.invalidateSignalStates(QThread::currentThreadId());

		return;
	}

	void TcpSignalClient::onReplyTimeout()
	{
		writeWarning(QString("TcpSignalClient::onReplyTimeout() %1").arg(serverAddressPort1().addressPortStr()));
	}

	void TcpSignalClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		if (replyData == nullptr)
		{
			Q_ASSERT(replyData);
			return;
		}

		QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

		switch (requestID)
		{
		case ADS_GET_APP_SIGNAL_LIST_START:
			processSignalListStart(data);
			break;

		case ADS_GET_APP_SIGNAL_LIST_NEXT:
			processSignalListNext(data);
			break;

		case ADS_GET_APP_SIGNAL_PARAM:
			processSignalParam(data);
			break;

		case ADS_GET_APP_SIGNAL_STATE_CHANGES:
			processSignalStateChanges(data);
			break;

		case ADS_GET_APP_SIGNAL_STATE:
			processSignalState(data);
			break;

		case ADS_GET_DISCRETES_LOG:
			processSignalLog(data);
			break;

		default:
			Q_ASSERT(false);
			writeError(QString("Wrong requestID in TcpSignalClient::processReply(), %1").arg(requestID));

			resetToGetState(true);
		}

		return;
	}

	void TcpSignalClient::resetToGetSignalList()
	{
		QThread::msleep(RequestTimeIntervalMs);

		reset();

		m_lastSignalParamStartIndex = 0;
		m_lastSignalStateStartIndex = 0;

		requestSignalListStart();
		return;
	}

	void TcpSignalClient::resetToGetState(bool resetStateIndex)
	{
		QThread::msleep(RequestTimeIntervalMs);

		if (resetStateIndex == true)
		{
			m_lastSignalStateStartIndex = 0;
		}

		if (m_signalList.empty() == false)
		{
			requestSignalStateChanges();
		}
		else
		{
			// There is no signals state to request list again.
			//
			resetToGetSignalList();
		}

		return;
	}

	void TcpSignalClient::requestSignalListStart()
	{
		Q_ASSERT(isClearToSendRequest());
		sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
	}

	void TcpSignalClient::processSignalListStart(const QByteArray& data)
	{
		bool ok = tl_getSignalListStartReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (tl_getSignalListStartReply.error() != 0)
		{
			writeError(QString("processSignalListNext, error received: %1").arg(tl_getSignalListStartReply.error()));

			Q_ASSERT(tl_getSignalListStartReply.error() != 0);

			resetToGetSignalList();
			return;
		}

		writeMessage("----------------- processSignalListStart -----------------");
		if (tl_getSignalListStartReply.error() == 0)
		{
			writeMessage(QString("-- error: %1").arg(tl_getSignalListStartReply.error()));
		}
		else
		{
			writeError(QString("-- error: %1").arg(tl_getSignalListStartReply.error()));
		}
		writeMessage(QString("-- totalItemCount: %1").arg(tl_getSignalListStartReply.totalitemcount()));
		writeMessage(QString("-- partCount: %1").arg(tl_getSignalListStartReply.partcount()));
		writeMessage(QString("-- itemsPerPart: %1").arg(tl_getSignalListStartReply.itemsperpart()));

		if (tl_getSignalListStartReply.totalitemcount() == 0 || tl_getSignalListStartReply.partcount() == 0)
		{
			// There is no signals, useless but can be
			//
			Q_ASSERT(tl_getSignalListStartReply.totalitemcount() == 0);
			Q_ASSERT(tl_getSignalListStartReply.partcount() == 0);

			reset();

			// request params
			//
			requestSignalParam(0);
			return;
		}

		reset();
		m_signalList.reserve(tl_getSignalListStartReply.totalitemcount());

		requestSignalListNext(0);

		return;
	}

	void TcpSignalClient::requestSignalListNext(int part)
	{
		Q_ASSERT(isClearToSendRequest());

		// if all parts were requested then switch to next reply
		//
		if (part >= tl_getSignalListStartReply.partcount())
		{
			Q_ASSERT(std::ssize(m_signalList) == tl_getSignalListStartReply.totalitemcount());

			// Request params
			//
			requestSignalParam(0);
			return;
		}

		// Request part, partNo is set in processSignalListStart and is incremented in processSignalListNext
		//
		tl_getSignalListNextRequest.set_part(part);

		sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, tl_getSignalListNextRequest);
		return;
	}

	void TcpSignalClient::processSignalListNext(const QByteArray& data)
	{
		bool ok = tl_getSignalListNextReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (tl_getSignalListNextReply.error() != 0)
		{
			writeError(QString("processSignalListNext, error received: %1").arg(tl_getSignalListNextReply.error()));

			Q_ASSERT(tl_getSignalListNextReply.error() != 0);

			resetToGetSignalList();
			return;
		}

		if (tl_getSignalListNextReply.part() != tl_getSignalListNextRequest.part())
		{
			// Asked for one part but got different
			//
			Q_ASSERT(tl_getSignalListNextReply.part() == tl_getSignalListNextRequest.part());
			resetToGetSignalList();
			return;
		}

		writeMessage("----------------- processSignalListNext -----------------");
		if (tl_getSignalListNextReply.error() == 0)
		{
			writeMessage(QString("-- error: %1").arg(tl_getSignalListNextReply.error()));
		}
		else
		{
			writeError(QString("-- error: %1").arg(tl_getSignalListNextReply.error()));
		}
		writeMessage(QString("-- part: %1").arg(tl_getSignalListNextReply.part()));

		{
			for (int i = 0; i < tl_getSignalListNextReply.appsignalids_size(); i++)
			{
				Hash hash = ::calcHash(QString::fromStdString(tl_getSignalListNextReply.appsignalids(i)));
				m_signalList.push_back(hash);
			}
		}

		// Next request
		//
		requestSignalListNext(tl_getSignalListNextReply.part() + 1);

		return;
	}

	// AppSignalParam
	//
	void TcpSignalClient::requestSignalParam(int startIndex)
	{
		Q_ASSERT(isClearToSendRequest());
		m_lastSignalParamStartIndex = startIndex;

		if (startIndex == 0)
		{
			// theSignals.reset();	no need to reset signal list, signals will be just updates,
			// if new configuration arrives (ONLY than signal list can be changed) then all signals will be reset in
			// AdsConnection::configurationArrived
			//
		}

		if (startIndex >= std::ssize(m_signalList))
		{
			m_signalUpdater.notifySignalParamsUpdated();

			// Set flag that all signal params were loaded
			//
			m_signalParamsLoaded.store(true);

			// Clear bus signals hashes from total hashes list
			//
			for (const Hash& busHash : m_busSignalHashes)
			{
				std::erase_if(m_signalList,
							  [busHash](Hash hash)
							  {
								  return hash == busHash;
							  });
			}

			resetToGetState(true); // END OF RECEIVING SIGNALS PARAMS,
			// Here the new loop starts!!!
			return;
		}

		tl_getSignalParamRequest.mutable_signalhashes()->Clear();
		tl_getSignalParamRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_PARAM_MAX);

		for (int i = startIndex; i < startIndex + ADS_GET_APP_SIGNAL_PARAM_MAX && i < std::ssize(m_signalList); i++)
		{
			Hash signalHash = m_signalList[i];
			tl_getSignalParamRequest.add_signalhashes(signalHash);
		}

		sendRequest(ADS_GET_APP_SIGNAL_PARAM, tl_getSignalParamRequest);
		return;
	}

	void TcpSignalClient::processSignalParam(const QByteArray& data)
	{
		bool ok = tl_getSignalParamReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (tl_getSignalParamReply.error() != 0)
		{
			writeError(QString("processSignalParam, error received: %1").arg(tl_getSignalParamReply.error()));

			Q_ASSERT(tl_getSignalParamReply.error() != 0);

			resetToGetState(true);
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.reserve(tl_getSignalParamReply.appsignals_size());

		for (int i = 0; i < tl_getSignalParamReply.appsignals_size(); i++)
		{
			const ::Proto::AppSignal& protoSignal = tl_getSignalParamReply.appsignals(i);

			AppSignalParam& s = appSignals.emplace_back();
			s.load(protoSignal);

			if (s.hash() == 0 || s.appSignalId().isEmpty() == true)
			{
				Q_ASSERT(s.hash() != 0);
				Q_ASSERT(s.appSignalId().isEmpty() == false);

				appSignals.pop_back();
				continue;
			}

			if (s.isBus() == true)
			{
				m_busSignalHashes.insert(s.hash());
			}
		}

		m_signalUpdater.addSignals(appSignals, m_serverSettings.equipmentId);

		requestSignalParam(m_lastSignalParamStartIndex + ADS_GET_APP_SIGNAL_PARAM_MAX);

		return;
	}

	// AppSignalStateChanges
	//
	void TcpSignalClient::requestSignalStateChanges()
	{
		Q_ASSERT(isClearToSendRequest());

		sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, tl_getSignalStateChangesRequest);

		return;
	}

	void TcpSignalClient::processSignalStateChanges(const QByteArray& data)
	{
		if (bool ok = tl_getSignalStateChangesReply.ParseFromArray(data.constData(), static_cast<int>(data.size())); ok == false)
		{
			Q_ASSERT(ok);
			resetToGetState(true);
			return;
		}

		if (tl_getSignalStateChangesReply.error() != 0)
		{
			writeError(QString("processSignalStateChanges, error received: %1").arg(tl_getSignalStateChangesReply.error()));
			Q_ASSERT(tl_getSignalStateChangesReply.error() != 0);

			resetToGetState(true);
			return;
		}

		// Check that the server and client time is the same.
		//
		if (m_timeDiscrepancyCheckDate.isNull() == true || m_timeDiscrepancyCheckDate != QDate::currentDate())
		{
			const qint64 serverUtcTimeMs = tl_getSignalStateChangesReply.servertimeutc();
			const qint64 serverLocalTimeMs = tl_getSignalStateChangesReply.servertimelocal();

			checkTimeDiscrepancy(serverUtcTimeMs, serverLocalTimeMs);

			// Set flag that time was checked for this connection.
			//
			m_timeDiscrepancyCheckDate = QDate::currentDate();
		}

		// --
		//
		int signalStateCount = tl_getSignalStateChangesReply.appsignalstates_size();

		thread_local std::vector<AppSignalState> states;

		states.clear();
		states.reserve(signalStateCount);

		for (int i = 0; i < signalStateCount; i++)
		{
			const AppSignalState& state = states.emplace_back(tl_getSignalStateChangesReply.appsignalstates(i));
			Q_ASSERT(state.hash() != 0);

			if (m_signalStatesSet.contains(state.hash()) == false)
			{
				m_signalStatesSet.insert(state.hash()); // Mark signal as received at least once

				if (m_signalStatesSet.size() == m_signalList.size())
				{
					m_signalStatesLoaded.store(true);   // Notify that states of all signals are received
				}
			}
		}

		m_signalUpdater.setState(states, ::calcHash(m_serverSettings.equipmentId), QThread::currentThreadId());

		if (tl_getSignalStateChangesReply.pendingstatescount() >= ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// A lot of signals are in the event queue, request one more time.
			//
			requestSignalStateChanges();
		}
		else
		{
			// Update all signals
			//
			requestSignalState(m_lastSignalStateStartIndex + MaxStateRequestCount);
		}

		return;
	}

	// AppSignalState
	//
	void TcpSignalClient::requestSignalState(int startIndex)
	{
		Q_ASSERT(isClearToSendRequest());

		if (startIndex >= std::ssize(m_signalList))
		{
			startIndex = 0;
		}

		m_lastSignalStateStartIndex = startIndex;

		tl_getSignalStateRequest.mutable_signalhashes()->Clear();
		tl_getSignalStateRequest.mutable_signalhashes()->Reserve(MaxStateRequestCount);

		for (int i = startIndex; i < startIndex + MaxStateRequestCount && i < std::ssize(m_signalList); i++)
		{
			Hash signalHash = m_signalList[i];
			tl_getSignalStateRequest.add_signalhashes(signalHash);
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE, tl_getSignalStateRequest);
		return;
	}

	void TcpSignalClient::processSignalState(const QByteArray& data)
	{
		if (bool ok = tl_getSignalStateReply.ParseFromArray(data.constData(), static_cast<int>(data.size())); ok == false)
		{
			Q_ASSERT(ok);
			resetToGetState(true);
			return;
		}

		if (tl_getSignalStateReply.error() != 0)
		{
			writeError(QString("processSignalState, error received: %1").arg(tl_getSignalStateReply.error()));

			Q_ASSERT(tl_getSignalStateReply.error() != 0);

			resetToGetState(true);
			return;
		}

		int signalStateCount = tl_getSignalStateReply.appsignalstates_size();

		std::vector<AppSignalState> states;
		states.reserve(signalStateCount);

		for (int i = 0; i < signalStateCount; i++)
		{
			const AppSignalState& state = states.emplace_back(tl_getSignalStateReply.appsignalstates(i));
			Q_ASSERT(state.m_hash != 0);

			if (m_signalStatesSet.contains(state.hash()) == false)
			{
				m_signalStatesSet.insert(state.hash()); // Mark signal as received at least once

				if (m_signalStatesSet.size() == m_signalList.size())
				{
					m_signalStatesLoaded.store(true);   // Notify that states of all signals are received
				}
			}
		}

		m_signalUpdater.setState(states, ::calcHash(m_serverSettings.equipmentId), QThread::currentThreadId());

		requestSignalLog();
		return;
	}

	void TcpSignalClient::requestSignalLog()
	{
		Q_ASSERT(isClearToSendRequest());

		if (m_signalLog.enabled() == true)
		{
			sendRequest(ADS_GET_DISCRETES_LOG);
		}
		else
		{
			resetToGetState(false);
		}

		return;
	}

	void TcpSignalClient::processSignalLog(const QByteArray& data)
	{
		bool ok = tl_getDiscretesLogReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetState(false);
			return;
		}

#if 0
		int pendingRecordsCount = tl_getDiscretesLogReply.pendingrecordscount();
#endif

		std::vector<DiscretesLogRecord> records;

		std::transform(tl_getDiscretesLogReply.discreteslogrecord().begin(),
					   tl_getDiscretesLogReply.discreteslogrecord().end(),
					   std::back_inserter(records),
					   [](const Network::DiscretesLogRecord& logRecord)
					   {
						   DiscretesLogRecord record;
						   record.loadFromProto(logRecord);
						   return record;
					   });

		m_signalLog.add(m_serverSettings.equipmentId, records);
		m_signalLog.deleteUpTo(m_serverSettings.equipmentId, tl_getDiscretesLogReply.logfirstrecordid());

		resetToGetState(false);
		return;
	}

	bool TcpSignalClient::signalParamsLoaded() const
	{
		return m_signalParamsLoaded.load();
	}

	bool TcpSignalClient::signalStatesLoaded() const
	{
		return m_signalStatesLoaded.load();
	}

	const SoftwareEndpoint::AppDataService& TcpSignalClient::server() const
	{
		return m_serverSettings;
	}

	void TcpSignalClient::reset()
	{
		m_signalList.clear();
		m_busSignalHashes.clear();
		m_signalStatesSet.clear();

		m_signalParamsLoaded.store(false);
		m_signalStatesLoaded.store(false);
	}

	void TcpSignalClient::checkTimeDiscrepancy(qint64 serverUtcTimeMs, qint64 serverLocalTimeMs)
	{
		const qint64 serverTimeZoneDiff = serverLocalTimeMs - serverUtcTimeMs;

		// 1. UTC time is different?
		//
		{
			const qint64 limitMs = static_cast<qint64>(3 * 1'000) * 60; // 3 minutes.
			const qint64 utcTimeDiscrepancy = std::abs(serverUtcTimeMs - QDateTime::currentDateTime().toMSecsSinceEpoch());

			if (utcTimeDiscrepancy > limitMs)
			{
				auto clientUtcDateTime = QDateTime::fromMSecsSinceEpoch(QDateTime::currentDateTime().toMSecsSinceEpoch(), QTimeZone::UTC);
				auto serverUtcDateTime = QDateTime::fromMSecsSinceEpoch(serverUtcTimeMs, QTimeZone::UTC);

				writeWarning(QString("UTC time discrepancy detected (%1 seconds). Client UTC time %2, server UTC time %3.")
								 .arg(utcTimeDiscrepancy / 1000)
					.arg(DateTimeToString::dateTime(clientUtcDateTime, true /*with ms*/))
					.arg(DateTimeToString::dateTime(serverUtcDateTime, true /*with ms*/)));
			}
			else
			{
				writeMessage(QString("UTC Time discrepancy is about %1 ms.").arg(utcTimeDiscrepancy));
			}
		}

		// 2. Time zone is different?
		//
		{
			QDateTime clientCurrentTimeLocal = QDateTime::currentDateTime();
			const qint64 clientUtcMs = clientCurrentTimeLocal.toMSecsSinceEpoch();
			clientCurrentTimeLocal.setTimeZone(QTimeZone::UTC);
			const qint64 clientLocalMs = clientCurrentTimeLocal.toMSecsSinceEpoch();

			const qint64 clientTimeZoneDiff = clientLocalMs - clientUtcMs;

			qint64 timeZoneDiff = std::abs(serverTimeZoneDiff - clientTimeZoneDiff);

			if (timeZoneDiff != 0)
			{
				auto clientLocalDateTime = clientCurrentTimeLocal;
				auto serverLocalDateTime = QDateTime::fromMSecsSinceEpoch(serverLocalTimeMs, QTimeZone::UTC);

				writeWarning(QString("TimeZone discrepancy detected. Client local time %1, server local time %2.")
								 .arg(DateTimeToString::dateTime(clientLocalDateTime, true /*with ms*/))
								 .arg(DateTimeToString::dateTime(serverLocalDateTime, true /*with ms*/)));
			}
		}

		return;
	}

} // namespace ClientLib
