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
		m_signalStatesUpdated(handler.m_signalStatesUpdated),
		m_lists(handler.m_lists),
		m_timer(this),
		m_socket(this)
	{
		HostAddressPort ip = handler.m_gateway->gatewayIP1();

		if (ip.isSet() == true)
		{
			m_channelsInfo.emplace_back(ip);
		}

		ip = handler.m_gateway->gatewayIP2();

		if (ip.isSet() == true)
		{
			m_channelsInfo.emplace_back(ip);
		}
	}

	void IvsImpulseCommThreadWorker::onSendStateChanges()
	{
		sendStateChanges();
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
		sendStateChanges();			// at first flush all existing state changes
		periodicSendStates();
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

			for(auto& ci : m_channelsInfo)
			{
				m_socket.writeDatagram(m_sendBuffer, packetSize,
									   ci.gatewayIP.address(), ci.gatewayIP.port());

				ci.statesPacketsSentCount++;

				if ((ci.statesPacketsSentCount % 10) == 0)
				{
					qDebug() << C_STR(QString("State packets send to %1: %3").
									  arg(ci.gatewayIP.addressPortStr()).
									  arg(ci.statesPacketsSentCount));
				}
			}
		}

		m_signalStatesUpdated = false;
	}

	void IvsImpulseCommThreadWorker::sendStateChanges()
	{
		QThread* thread = QThread::currentThread();

		IvsImpulseEventsPacket* packet = reinterpret_cast<IvsImpulseEventsPacket*>(m_sendBuffer);

		for(IvsImpulseListInfoShared& li : m_lists)
		{
			IvsImpulsePacketHeader& header = packet->header;

			header.systemID = static_cast<quint8>(m_gateway->systemID());
			header.dataType = static_cast<quint8>(li->info->dataTypeLetter());
			header.listID = static_cast<quint8>(li->info->listNo());
			header.listVersion = static_cast<quint8>(m_gateway->listsVersion());
			header.firstParamIndex = 0;				// sign of events packet

			qint64 packetSize = sizeof(IvsImpulsePacketHeader);

			//

			li->stateChangesMutex.lock(thread);

			int writtenParamCount = 0;
			qint64 baseTime_ms = 0;

			packetSize += writeStateChangesToPacket(li,
													&packet->signalEvents,
													li->info->dataType(),
													baseTime_ms,
													li->stateChangesToRead,
													writtenParamCount);

			Q_ASSERT(writtenParamCount == static_cast<int>(li->stateChangesToRead.size()));

			li->stateChangesToRead.clear();
			li->minTime.clear();

			li->stateChangesMutex.unlock(thread);

			//

			header.paramCount = static_cast<quint16>(writtenParamCount);

			header.time = static_cast<qint32>(baseTime_ms / 1000);	// milliseconds -> seconds

			for(auto& ci : m_channelsInfo)
			{
				m_socket.writeDatagram(m_sendBuffer, packetSize,
									   ci.gatewayIP.address(), ci.gatewayIP.port());

				ci.statesPacketsSentCount++;

				if ((ci.statesPacketsSentCount % 10) == 0)
				{
					qDebug() << C_STR(QString("State packets send to %1: %3").
									  arg(ci.gatewayIP.addressPortStr()).
									  arg(ci.statesPacketsSentCount));
				}
			}
		}

	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket(IvsImpulseStatesPacket* packet,
														E::SignalListDataType dataType,
														int startIndex, int size,
														int& paramCount, qint64& time)
	{
		TEST_PTR_RETURN_VALUE(packet, 0);

		switch(dataType)
		{
		case E::SignalListDataType::Analog_A:
			return writeStatesToPacket_A(&packet->states_A, startIndex, size, paramCount, time);

		case E::SignalListDataType::Discrete_B:
			return writeStatesToPacket_B(&packet->states_B, startIndex, size, paramCount, time);

		case E::SignalListDataType::Discrete_D:
			return writeStatesToPacket_D(&packet->states_D, startIndex, size, paramCount, time);

		default:
			Q_ASSERT(false);
		}

		return 0;
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
															IvsImpulseSignalEvent* event,
															E::SignalListDataType dataType,
															qint64 baseTime_ms,
															const std::vector<GatewayAppSignalState>& stateChanges,
															int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(event, 0);

		::E::TimeType timeType = m_gateway->timeType();
		paramCount = 0;
		int dataSize = 0;

		qint64 time = 0;

		for(const GatewayAppSignalState& st : stateChanges)
		{
			auto it = li->hashToListIndex.find(st.curState.hash);

			if (it == li->hashToListIndex.end())
			{
				Q_ASSERT(false);
				continue;
			}

			event->indexInList = static_cast<quint16>(it->second);

			switch(timeType)
			{
			case ::E::TimeType::Plant:	time = st.curState.plantTime(); break;
			case ::E::TimeType::System:	time = st.curState.systemTime(); break;
			case ::E::TimeType::Local:	time = st.curState.localTime(); break;
			default: Q_ASSERT(false);
			}

			qint64 timeOffset = time - baseTime_ms;

			if (timeOffset < 0)
			{
				Q_ASSERT(false);
				timeOffset = 0;
			}

			event->timeOffset = static_cast<quint16>(timeOffset);

			switch(dataType)
			{
			case E::SignalListDataType::Analog_A:
				event->prevCode_A = getAnalogStateCodeA(st.prevState);
				event->newCode_A = getAnalogStateCodeA(st.curState);
				break;

			case E::SignalListDataType::Discrete_D:
			case E::SignalListDataType::Discrete_B:
				event->prevState_D = getDiscreteStateD(st.prevState);
				event->newState_D = getDiscreteStateD(st.curState);
				break;

			default: Q_ASSERT(false);
			}

			event++;

			paramCount++;
			dataSize += sizeof(IvsImpulseSignalEvent);
		}

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

		for(auto worker : m_workerList)
		{
			SimpleThread::connect(client,
								  &AppDataServiceClient::sendStateChanges,
								  dynamic_cast<IvsImpulseCommThreadWorker*>(worker),
								  &IvsImpulseCommThreadWorker::onSendStateChanges);
		}
	}
}

