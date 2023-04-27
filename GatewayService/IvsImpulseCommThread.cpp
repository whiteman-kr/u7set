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
		periodicSendStates();
	}

	void IvsImpulseCommThreadWorker::periodicSendStates()
	{
		IvsImpulseStatesPacket* packet = reinterpret_cast<IvsImpulseStatesPacket*>(m_sendBuffer);

		for(IvsImpulseListInfo& li : m_lists)
		{
			IvsImpulsePacketHeader& header = packet->header;

			header.systemID = static_cast<quint8>(m_gateway->systemID());
			header.dataType = static_cast<quint8>(li.info->dataTypeLetter());
			header.listID = static_cast<quint8>(li.info->listNo());
			header.listVersion = static_cast<quint8>(m_gateway->listsVersion());
			header.firstParamIndex = 1;

			header.time = 0; // ??????

			qint64 packetSize = sizeof(IvsImpulsePacketHeader);

			int writtenParamCount = 0;

			packetSize += writeStatesToPacket(packet, li.info->dataType(),
											  li.startIndex, li.size, writtenParamCount);

			Q_ASSERT(writtenParamCount == li.size);

			header.paramCount = static_cast<quint16>(writtenParamCount);

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
														int startIndex, int size, int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(packet, 0);

		switch(dataType)
		{
		case E::SignalListDataType::Analog_A:
			return writeStatesToPacket_A(&packet->states_A, startIndex, size, paramCount);

		case E::SignalListDataType::Discrete_B:
			return writeStatesToPacket_B(&packet->states_B, startIndex, size, paramCount);

		case E::SignalListDataType::Discrete_D:
			return writeStatesToPacket_D(&packet->states_D, startIndex, size, paramCount);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	int IvsImpulseCommThreadWorker::writeStatesToPacket_A(AnalogState_A* states,
														  int startIndex, int size, int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

		paramCount = 0;
		int dataSize = 0;

		AnalogState_A* stateA = states;

		for(int i = startIndex; i < startIndex + size; i++)
		{
			SimpleAppSignalState st = m_states[i].getState();

			stateA->value = static_cast<float>(st.value);
			stateA->stateCode = getAnalogStateCodeA(st.flags);

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
														  int startIndex, int size, int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

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
			SimpleAppSignalState st = m_states[i].getState();

			*statesB |= (st.value == 0 ? 0 : 1) << bitNo;
			*validityB |= (st.isValid() == true ? 0 : 1) << bitNo;

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
														  int startIndex, int size, int& paramCount)
	{
		TEST_PTR_RETURN_VALUE(states, 0);

		paramCount = 0;
		int dataSize = 0;

		DiscreteState_D* stateD = states;

		for(int i = startIndex; i < startIndex + size; i++)
		{
			SimpleAppSignalState st = m_states[i].getState();

			stateD->value = st.value == 0 ? 0 : 1;
			stateD->notValid = st.isValid() == true ? 0 : 1;

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

	AnalogStateCode_A IvsImpulseCommThreadWorker::getAnalogStateCodeA(::AppSignalStateFlags flags) const
	{
		AnalogStateCode_A code;

		code.flag.notValid = !flags.valid;

		if (flags.aboveHighLimit)
		{
			code.flag.deviationCode = DEV_CODE_A_VRG;
		}

		if (flags.belowLowLimit)
		{
			code.flag.deviationCode = DEV_CODE_A_NRG;
		}

		return code;
	}
}

