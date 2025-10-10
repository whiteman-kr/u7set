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
		m_log(handler.m_log),
		m_gateway(handler.m_gateway),
		m_appSignals(handler.m_appSignals),
		m_states(handler.m_states),
		m_signalStatesUpdated(handler.m_signalStatesUpdated),
		m_lists(handler.m_lists),
		m_logGatewayPackets(handler.m_logGatewayPackets),
		m_timer(this)
	{
		HostAddressPort localIP = handler.m_gateway->localGatewayIP1();
		HostAddressPort remoteIP = handler.m_gateway->remoteGatewayIP1();

		if (localIP.isSet() == true && remoteIP.isSet() == true)
		{
			m_channelsInfo.emplace_back(localIP, remoteIP);
		}

		localIP = handler.m_gateway->localGatewayIP2();
		remoteIP = handler.m_gateway->remoteGatewayIP2();

		if (localIP.isSet() == true && remoteIP.isSet() == true)
		{
			m_channelsInfo.emplace_back(localIP, remoteIP);
		}
	}

	void IvsImpulseCommThreadWorker::onSendStateChanges()
	{
		if (isWorkableSocketExists() == true)
		{
			sendStateChanges();
		}
	}

	void IvsImpulseCommThreadWorker::onThreadStarted()
	{
		if (m_logGatewayPackets == true)
		{
			m_packetsLog = std::make_shared<CircularLogger>();
			circularLoggerInit(m_packetsLog, QString("%1_Packets").arg(m_gateway->gatewayID()),
							   QString(), 10, 50);
			m_packetsLog->setLogCodeInfo(false);

			m_logStartTime = QDateTime::currentMSecsSinceEpoch();
		}

		//

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
		bool workableSocketExists = tryCreateSockets();

		if (workableSocketExists == true)
		{
			sendStateChanges();			// at first flush all existing state changes
			periodicSendStates();
		}

		checkLogTime();
	}

	bool IvsImpulseCommThreadWorker::tryCreateSockets()
	{
		bool workableSocketExists = false;

		for(GatewayChannelInfo& ci : m_channelsInfo)
		{
			if (ci.socket != nullptr)
			{
				workableSocketExists = true;
				continue;
			}

			workableSocketExists |= ci.tryCreateSocket(m_log);
		}

		return workableSocketExists;
	}

	bool IvsImpulseCommThreadWorker::isWorkableSocketExists() const
	{
		for(auto& ci : m_channelsInfo)
		{
			if (ci.socket != nullptr)
			{
				return true;
			}
		}

		return false;
	}

	void IvsImpulseCommThreadWorker::periodicSendStates()
	{
		if (m_signalStatesUpdated == false)
		{
			return;
		}

		IvsImpulseStatesPacket* packet = reinterpret_cast<IvsImpulseStatesPacket*>(m_sendBuffer);

		for(IvsImpulseListInfoShared& li : m_lists)
		{
			IvsImpulsePacketHeader& header = packet->header;

			header.systemID = static_cast<quint8>(m_gateway->systemID());
			header.dataType = static_cast<quint8>(li->info->dataTypeLetter());
			header.listID = static_cast<quint8>(li->info->listNo());
			header.listVersion = static_cast<quint8>(m_gateway->listsVersion());
			header.firstParamIndex = 1;

			qint64 packetSize = sizeof(IvsImpulsePacketHeader);

			int writtenParamCount = 0;
			qint64 time = 0;

			packetSize += writeStatesToPacket(packet, li->info->dataType(),
											  li->startIndex, li->size,
											  writtenParamCount, time);

			Q_ASSERT(writtenParamCount == li->size);

			header.paramCount = static_cast<quint16>(writtenParamCount);

			// time in milliseconds
			// header.time in seconds
			//
			header.time = static_cast<qint32>(time / 1000);

			sendPacket(m_sendBuffer, packetSize, false);
		}

		m_signalStatesUpdated = false;
	}

	void IvsImpulseCommThreadWorker::sendStateChanges()
	{
		IvsImpulseEventsPacket* packet = reinterpret_cast<IvsImpulseEventsPacket*>(m_sendBuffer);

		for(IvsImpulseListInfoShared& li : m_lists)
		{
			if (li->hasStateChanges() == false)
			{
				continue;
			}

			IvsImpulsePacketHeader& header = packet->header;

			header.systemID = static_cast<quint8>(m_gateway->systemID());
			header.dataType = static_cast<quint8>(li->info->dataTypeLetter());
			header.listID = static_cast<quint8>(li->info->listNo());
			header.listVersion = static_cast<quint8>(m_gateway->listsVersion());
			header.firstParamIndex = 0;				// sign of events packet

			qint64 packetSize = sizeof(IvsImpulsePacketHeader);

			//

			li->stateChangesMutex.lock();

			int writtenParamCount = 0;
			qint64 baseTime_ms = -1;

			packetSize += writeStateChangesToPacket(li,
													&packet->signalEvents,
													li->info->dataType(),
													baseTime_ms,
													li->stateChangesToRead,
													writtenParamCount);
			li->stateChangesToRead.clear();
			li->stateChangesMutex.unlock();

			baseTime_ms = convertTimeToUTC(baseTime_ms, m_gateway->timeType());

			//

			header.paramCount = static_cast<quint16>(writtenParamCount);

			header.time = static_cast<qint32>(baseTime_ms / 1000);	// milliseconds -> seconds

			sendPacket(m_sendBuffer, packetSize, true);
		}
	}

	void IvsImpulseCommThreadWorker::sendPacket(const char* packet, qint64 packetSize, bool eventsPacket)
	{
		TEST_PTR_RETURN(packet);

		for(GatewayChannelInfo& ci : m_channelsInfo)
		{
			if (ci.socket == nullptr)
			{
				continue;
			}

			qint64 res = ci.socket->writeDatagram(packet, packetSize,
								   ci.remoteGatewayIP.address(), ci.remoteGatewayIP.port());

			if (res == -1)
			{
				DEBUG_LOG_ERR(m_log, QString("Error send packet to %1 via %2 (%3). Socket closed.").
										arg(ci.remoteGatewayIP.addressPortStr()).
										arg(ci.localGatewayIP.addressPortStr()).
										arg(ci.socket->errorString()));
				ci.clearSocket();

				continue;
			}

			if (eventsPacket == true)
			{
				if (m_packetsLog != nullptr)
				{
					logEventsPacket(ci, packet);
				}

				ci.eventPacketsSentCount++;

				if ((ci.eventPacketsSentCount % 20) == 0)
				{
					qDebug() << C_STR(QString("Events packets send to %1 via %2: %3").
									  arg(ci.remoteGatewayIP.addressPortStr()).
									  arg(ci.localGatewayIP.addressPortStr()).
									  arg(ci.eventPacketsSentCount));
				}
			}
			else
			{
				if (m_packetsLog != nullptr)
				{
					logPeriodicPacket(ci, packet);
				}

				ci.statesPacketsSentCount++;

				if ((ci.statesPacketsSentCount % 20) == 0)
				{
					qDebug() << C_STR(QString("Periodic packets send to %1 via %2: %3").
									  arg(ci.remoteGatewayIP.addressPortStr()).
									  arg(ci.localGatewayIP.addressPortStr()).
									  arg(ci.statesPacketsSentCount));
				}
			}
		}

	}

	void IvsImpulseCommThreadWorker::logEventsPacket(const GatewayChannelInfo& ci, const char* packet)
	{
		TEST_PTR_RETURN(m_packetsLog);
		TEST_PTR_RETURN(packet);

		auto toBin = [](quint8 b) -> QString
		{
			QString binCode;

			for(int i = 0; i < 8; i++)
			{
				binCode += (b & 0x80 ? "1" : "0");
				b <<= 1;
			}

			return binCode;
		};

		const IvsImpulseEventsPacket* eventPacket = reinterpret_cast<const IvsImpulseEventsPacket*>(packet);
		const IvsImpulsePacketHeader& header = eventPacket->header;
		const IvsImpulseSignalEvent* event = &eventPacket->signalEvents;

		QString&& str = QString("send to %1 events Time=%2, SID=%3, DT=%4, LID=%5, LV=%6, FI=%7, PCnt=%8, PacketNo=%9: ").
				arg(ci.remoteGatewayIP.addressPortStr()).
				arg(formatTime(header.time)).
				arg(header.systemID).arg(QChar(header.dataType)).arg(header.listID).arg(header.listVersion).
				arg(header.firstParamIndex).arg(header.paramCount).
				arg(*reinterpret_cast<const quint16*>(event + header.paramCount));

		for(quint16 i = 0; i < header.paramCount; i++, event++)
		{
			str += QString("[indx=%1, tm=%2, p=%3, n=%4]").
						arg(event->indexInList).arg(event->timeOffset).
						arg(toBin(event->prevCode_A.allFlags)).arg(toBin(event->newCode_A.allFlags));
		}

		DEBUG_LOG_MSG(m_packetsLog, str);
	}

	void IvsImpulseCommThreadWorker::logPeriodicPacket(const GatewayChannelInfo& ci, const char* packet)
	{
		TEST_PTR_RETURN(m_packetsLog);
		TEST_PTR_RETURN(packet);

		const IvsImpulseStatesPacket* statesPacket = reinterpret_cast<const IvsImpulseStatesPacket*>(packet);
		const IvsImpulsePacketHeader& header = statesPacket->header;

		QString&& str = QString("send to %1 period Time=%2, SID=%3, DT=%4, LID=%5, LV=%6, FI=%7, PCnt=%8").
				arg(ci.remoteGatewayIP.addressPortStr()).
				arg(formatTime(header.time)).
				arg(header.systemID).arg(QChar(header.dataType)).arg(header.listID).arg(header.listVersion).
				arg(header.firstParamIndex).arg(header.paramCount);

		DEBUG_LOG_MSG(m_packetsLog, str);
	}

	void IvsImpulseCommThreadWorker::checkLogTime()
	{
		if (m_packetsLog == nullptr)
		{
			return;
		}

		m_checkLogTimeCtr++;

		if (m_checkLogTimeCtr < 10)
		{
			return;
		}

		m_checkLogTimeCtr = 0;

		if (QDateTime::currentMSecsSinceEpoch() - m_logStartTime > 2 * 60 * 60 * 1000)		// 2 Hours
		{
			m_packetsLog.reset();
		}
	}

	qint64 IvsImpulseCommThreadWorker::convertTimeToUTC(quint64 time, ::E::TimeType timeType) const
	{
		switch(timeType)
		{
		case ::E::TimeType::Local:
		case ::E::TimeType::Plant:
			{
				// convert local time to UTC0
				//
				QDateTime dt = QDateTime::fromMSecsSinceEpoch(time, TIME_ZONE_UTC);
				dt.setTimeZone(TIME_ZONE_LOCAL);
				time = dt.toMSecsSinceEpoch();
			}
			break;

		case ::E::TimeType::System:
			// no conversion required
			break;

		default:
			Q_ASSERT(false);

		}

		return time;
	}

	QString IvsImpulseCommThreadWorker::formatTime(quint32 seconds)
	{
		QDateTime dt;

		dt.setTimeZone(TIME_ZONE_UTC);
		dt.setSecsSinceEpoch(seconds);

		QDate d = dt.date();
		QTime t = dt.time();

		return QString("%1.%2.%3 %4:%5:%6").
						arg(d.year()).
						arg(d.month(), 2, 10, Latin1Char::ZERO).
						arg(d.day(), 2, 10, Latin1Char::ZERO).
						arg(t.hour(), 2, 10, Latin1Char::ZERO).
						arg(t.minute(), 2, 10, Latin1Char::ZERO).
						arg(t.second(), 2, 10, Latin1Char::ZERO);
	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket(IvsImpulseStatesPacket* packet,
														E::SignalListDataType dataType,
														int startIndex, int size,
														int& paramCount, qint64& time)
	{
		TEST_PTR_RETURN_VALUE(packet, 0);

		int dataSize = 0;

		switch(dataType)
		{
		case E::SignalListDataType::Analog_A:
			dataSize = writeStatesToPacket_A(&packet->states_A, startIndex, size, paramCount, time);
			break;

		case E::SignalListDataType::Discrete_B:
			dataSize = writeStatesToPacket_B(&packet->states_B, startIndex, size, paramCount, time);
			break;

		case E::SignalListDataType::Discrete_D:
			dataSize = writeStatesToPacket_D(&packet->states_D, startIndex, size, paramCount, time);
			break;

		default:
			Q_ASSERT(false);
		}

		if (time == 0)
		{
			time = QDateTime::currentMSecsSinceEpoch();
		}
		else
		{
			time = convertTimeToUTC(time, m_gateway->timeType());
		}

		return dataSize;
	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket_A(AnalogState_A* states,
														  int startIndex, int size,
														  int& paramCount, qint64& time)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

		::E::TimeType timeType = m_gateway->timeType();
		paramCount = 0;
		time = 0;
		int dataSize = 0;

		AnalogState_A* stateA = states;

		for(int i = startIndex; i < startIndex + size; i++)
		{
			const SimpleAppSignalState& st = m_states[i].getState();

			stateA->value = static_cast<float>(st.value);
			stateA->stateCode = getAnalogStateCodeA(st);

			switch(timeType)
			{
			case ::E::TimeType::Plant:	time = std::max(time, st.time.plant.timeStamp); break;
			case ::E::TimeType::System:	time = std::max(time, st.time.system.timeStamp); break;
			case ::E::TimeType::Local:	time = std::max(time, st.time.local.timeStamp); break;
			default: Q_ASSERT(false);
			}

			stateA++;

			paramCount++;
			dataSize += sizeof(AnalogState_A);

			if (dataSize + sizeof(AnalogState_A) > IVS_IMPULSE_DATA_SECTION_MAX_SIZE)
			{
				Q_ASSERT(false);
				break;
			}
		}

		return dataSize;
	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket_B(DiscreteState_B* states,
														  int startIndex, int size,
														  int& paramCount, qint64& time)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

		::E::TimeType timeType = m_gateway->timeType();
		paramCount = 0;
		int dataSize = 0;

		const int MAX_B_PARAM_COUNT = (IVS_IMPULSE_DATA_SECTION_MAX_SIZE / 2) * 8;

		size = std::min(size, MAX_B_PARAM_COUNT);

		int statesArraySize = ((size - 1) / 8) + 1;

		dataSize = statesArraySize * 2;		// states array size + validity array size

		quint8* statesB = reinterpret_cast<quint8*>(states);
		quint8* validityB = statesB + statesArraySize;

		int bitNo = 0;

		for(int i = startIndex; i < startIndex + size; i++)
		{
			const SimpleAppSignalState& st = m_states[i].getState();

			*statesB |= (st.value == 0 ? 0 : 1) << bitNo;
			*validityB |= (st.isValid() == true ? 0 : 1) << bitNo;

			switch(timeType)
			{
			case ::E::TimeType::Plant:	time = std::max(time, st.time.plant.timeStamp); break;
			case ::E::TimeType::System:	time = std::max(time, st.time.system.timeStamp); break;
			case ::E::TimeType::Local:	time = std::max(time, st.time.local.timeStamp); break;
			default: Q_ASSERT(false);
			}

			paramCount++;

			bitNo++;

			if (bitNo >= 8)
			{
				bitNo = 0;
				statesB++;
				validityB++;
			}
		}

		return dataSize;
	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket_D(DiscreteState_D* states,
														  int startIndex, int size,
														  int& paramCount, qint64& time)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

		::E::TimeType timeType = m_gateway->timeType();
		paramCount = 0;
		int dataSize = 0;

		DiscreteState_D* stateD = states;

		for(int i = startIndex; i < startIndex + size; i++)
		{
			const SimpleAppSignalState& st = m_states[i].getState();

			*stateD = getDiscreteStateD(st);

			switch(timeType)
			{
			case ::E::TimeType::Plant:	time = std::max(time, st.time.plant.timeStamp); break;
			case ::E::TimeType::System:	time = std::max(time, st.time.system.timeStamp); break;
			case ::E::TimeType::Local:	time = std::max(time, st.time.local.timeStamp); break;
			default: Q_ASSERT(false);
			}

			stateD++;

			paramCount++;
			dataSize += sizeof(DiscreteState_D);

			if (dataSize + sizeof(DiscreteState_D) > IVS_IMPULSE_DATA_SECTION_MAX_SIZE)
			{
				Q_ASSERT(false);
				break;
			}
		}

		return dataSize;
	}

	int IvsImpulseCommThreadWorker::writeStateChangesToPacket(std::shared_ptr<IvsImpulseListInfo>& li,
															IvsImpulseSignalEvent* events,
															E::SignalListDataType dataType,
															qint64& baseTime_ms,
															const std::vector<GatewayAppSignalState>& stateChanges,
															int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(events, 0);

		::E::TimeType timeType = m_gateway->timeType();
		paramCount = 0;
		int dataSize = 0;

		qint64 time = 0;

		m_eventsTimes.clear();
		baseTime_ms = std::numeric_limits<qint64>::max();

		IvsImpulseSignalEvent* eventPtr = events;

		for(const GatewayAppSignalState& st : stateChanges)
		{
			auto it = li->hashToListIndexes.find(st.curState.hash);

			if (it == li->hashToListIndexes.end())
			{
				Q_ASSERT(false);
				continue;
			}

			switch(timeType)
			{
			case ::E::TimeType::Plant:	time = st.curState.plantTime(); break;
			case ::E::TimeType::System:	time = st.curState.systemTime(); break;
			case ::E::TimeType::Local:	time = st.curState.localTime(); break;
			default: Q_ASSERT(false);
			}

			baseTime_ms = std::min(baseTime_ms, time);							// detect minimal time in events

			std::vector<int>& indexes = it->second;

			for(int indexInList : indexes)
			{
				eventPtr->indexInList = static_cast<quint16>(indexInList);

				m_eventsTimes.push_back(time);									// save events times

				switch(dataType)
				{
				case E::SignalListDataType::Analog_A:
					eventPtr->prevCode_A = getAnalogStateCodeA(st.prevState);
					eventPtr->newCode_A = getAnalogStateCodeA(st.curState);
					break;

				case E::SignalListDataType::Discrete_D:
				case E::SignalListDataType::Discrete_B:
					eventPtr->prevState_D = getDiscreteStateD(st.prevState);
					eventPtr->newState_D = getDiscreteStateD(st.curState);
					break;

				default: Q_ASSERT(false);
				}

				eventPtr++;

				paramCount++;
				dataSize += sizeof(IvsImpulseSignalEvent);
			}
		}

		eventPtr = events;

		baseTime_ms /= 1000;
		baseTime_ms *= 1000;

		// writing events time offsets
		//
		for(int i = 0; i < paramCount; i++)
		{
			qint64 timeOffset = m_eventsTimes[i] - baseTime_ms;

			if (timeOffset < 0)
			{
				Q_ASSERT(false);
				timeOffset = 0;
			}

			eventPtr->timeOffset = static_cast<quint16>(timeOffset);
			eventPtr++;
		}

		quint16* packetNoPtr = reinterpret_cast<quint16*>(eventPtr);

		*packetNoPtr = li->eventsPacketNo++;

		dataSize += sizeof(quint16);

		return dataSize;
	}

	AnalogStateCode_A IvsImpulseCommThreadWorker::getAnalogStateCodeA(const SimpleAppSignalState& state) const
	{
		const ::AppSignalStateFlags& flags = state.flags;

		AnalogStateCode_A codeA;

		codeA.flag.notValid = !flags.valid;

		if (flags.aboveHighLimit)
		{
			codeA.flag.deviationCode = DEV_CODE_A_VRG;
		}

		if (flags.belowLowLimit)
		{
			codeA.flag.deviationCode = DEV_CODE_A_NRG;
		}

		return codeA;
	}

	DiscreteState_D IvsImpulseCommThreadWorker::getDiscreteStateD(const SimpleAppSignalState& state) const
	{
		DiscreteState_D stateD;

		stateD.value = state.value == 0 ? 0 : 1;
		stateD.notValid = state.isValid() == true ? 0 : 1;

		return stateD;
	}

	// --------------------------------------------------------------------------------------
	//
	//  IvsImpulseCommThreadWorker::GatewayChannelInfo struct implementation
	//
	// --------------------------------------------------------------------------------------

	bool IvsImpulseCommThreadWorker::GatewayChannelInfo::tryCreateSocket(CircularLoggerShared log)
	{
		if (socket != nullptr)
		{
			Q_ASSERT(false);
			return true;			// Ok!
		}

		if (localGatewayIP.isSet() == false ||
			remoteGatewayIP.isSet() == false)
		{
			return false;
		}

		qint64 now = QDateTime::currentMSecsSinceEpoch();

		if (now - prevTryCreateSocketTime < TRY_CREATE_SOCKET_INTERVAL_MS)
		{
			return false;
		}

		prevTryCreateSocketTime = now;

		bool result = false;

		socket = new QUdpSocket;

		result = socket->bind(localGatewayIP.address(), localGatewayIP.port(), QAbstractSocket::ShareAddress);

		if (result == true)
		{
			DEBUG_LOG_MSG(log, QString("Socket created and bound to %1 (remote gateway IP is %2)").
										arg(localGatewayIP.addressPortStr()).
										arg(remoteGatewayIP.addressPortStr()));
		}
		else
		{
			DEBUG_LOG_ERR(log, QString("Socket can't bind to %1").arg(localGatewayIP.addressPortStr()));

			delete socket;
			socket = nullptr;
		}

		return result;
	}

	// --------------------------------------------------------------------------------------
	//
	//  IvsImpulseCommThread class implementation
	//
	// --------------------------------------------------------------------------------------

	IvsImpulseCommThread::IvsImpulseCommThread(IvsImpulseHandler& handler)
	{
		addWorker(new IvsImpulseCommThreadWorker(handler));
	}

	void IvsImpulseCommThread::connect(AppDataServiceClient* client)
	{
		TEST_PTR_RETURN(client);

		for(auto worker : m_workers)
		{
			SimpleThread::connect(client,
								  &AppDataServiceClient::sendStateChanges,
								  dynamic_cast<IvsImpulseCommThreadWorker*>(worker),
								  &IvsImpulseCommThreadWorker::onSendStateChanges);
		}
	}
}

