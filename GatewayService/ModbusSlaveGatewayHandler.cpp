#include <string.h>
#include <Network.pb.h>

#include "ModbusSlaveGatewayHandler.h"
#include "ModbusTcpSlaveThread.h"
#include "ModbusUdpSlaveThread.h"

#include "Float16.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::ModbusTcpSlaveHandler::SignalState struct implementation
	//
	// ---------------------------------------------------------------------------------

	ModbusSlaveHandler::SignalState::SignalState(const ModbusFormat& frmt, const Address16& registerNo)
	{
		format = frmt;
		regNo = registerNo;
	}

	// ---------------------------------------------------------------------------------
	//
	//	Gateway::ModbusTcpSlaveHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	ModbusSlaveHandler::ModbusSlaveHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 ModbusSlaveGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(gateway->gatewayID(), swInfo, settings, log, logGatewayPackets),
		m_softwareInfo(swInfo),
		m_appDataService1(settings.appDataService1.address),
		m_appDataService2(settings.appDataService2.address),
		m_gateway(gateway),
		m_appSignals(appSignals)
	{
	}

	ModbusSlaveHandler::~ModbusSlaveHandler()
	{
	}

	void ModbusSlaveHandler::run()
	{
		init();

		m_appDataServiceClientThread =
				new AppDataServiceClientThread( m_softwareInfo,
												m_appDataService1,
												m_appDataService2,
												QString("GatewayService %1").arg(m_softwareInfo.equipmentID()),
												this, m_log);
		m_appDataServiceClientThread->start();

		switch(modbusMode())
		{
		case E::ModbusMode::TCP:

			if (listeningIP1().isSet() == true)
			{
				m_tcpSlaveThread1 = new TcpSlaveThread(listeningIP1(), *this);
				m_tcpSlaveThread1->start();
			}

			if (listeningIP2().isSet() == true)
			{
				m_tcpSlaveThread2 = new TcpSlaveThread(listeningIP2(), *this);
				m_tcpSlaveThread2->start();
			}
			break;

		case E::ModbusMode::UDP_ASCII:

			if (listeningIP1().isSet() == true)
			{
				m_udpSlaveThread1 = new UdpSlaveThread(listeningIP1(), *this);
				m_udpSlaveThread1->start();
			}

			if (listeningIP2().isSet() == true)
			{
				m_udpSlaveThread2 = new UdpSlaveThread(listeningIP2(), *this);
				m_udpSlaveThread2->start();
			}
			break;

		default:
			Q_ASSERT(false);
		}
	}

	void ModbusSlaveHandler::shutdown()
	{
		if (m_udpSlaveThread1 != nullptr)
		{
			m_udpSlaveThread1->stop();
			delete m_udpSlaveThread1;
			m_udpSlaveThread1 = nullptr;
		}

		if (m_udpSlaveThread2 != nullptr)
		{
			m_udpSlaveThread2->stop();
			delete m_udpSlaveThread2;
			m_udpSlaveThread2 = nullptr;
		}

		if (m_tcpSlaveThread1 != nullptr)
		{
			m_tcpSlaveThread1->stop();
			delete m_tcpSlaveThread1;
			m_tcpSlaveThread1 = nullptr;
		}

		if (m_tcpSlaveThread2 != nullptr)
		{
			m_tcpSlaveThread2->stop();
			delete m_tcpSlaveThread2;
			m_tcpSlaveThread2 = nullptr;
		}

		if (m_appDataServiceClientThread != nullptr)
		{
			m_appDataServiceClientThread->quitAndWait();
			delete m_appDataServiceClientThread;
			m_appDataServiceClientThread = nullptr;
		}

		Handler::shutdown();
	}

	void ModbusSlaveHandler::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getRequiredSignalsHashes(hashes);
	}

	void ModbusSlaveHandler::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getEventSignalsHashes(hashes);
	}

	void ModbusSlaveHandler::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
	{
		int statesCount = getStatesReply.appsignalstates_size();

		for(int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = getStatesReply.appsignalstates(i);

			Hash hash = state.hash();

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				continue;
			}

			std::list<SignalState>& states = it->second;

			for(SignalState& st : states)
			{
				st.value = state.value();
			}
		}

		updateAllRegisters();
	}

	void ModbusSlaveHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
		int statesCount = getStateChangesReply.appsignalstates_size();

		m_hashesToUpdate.clear();

		for(int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = getStateChangesReply.appsignalstates(i).curstate();

			Hash hash = state.hash();

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			m_hashesToUpdate.emplace(hash);

			std::list<SignalState>& states = it->second;

			for(SignalState& st : states)
			{
				st.value = state.value();
			}
		}

		updateRegisters(m_hashesToUpdate);
	}

	E::ModbusMode ModbusSlaveHandler::modbusMode() const
	{
		return m_gateway->modbusMode();
	}

	int ModbusSlaveHandler::modbusDeviceID() const
	{
		return m_gateway->modbusDeviceID();
	}

	int ModbusSlaveHandler::getRegistersValues(int regsStartAddr, int regsCount,
												RegisterValue* destBuffer, int maxRegsCount,
												QThread* thread)
	{
		if (maxRegsCount < regsCount)
		{
			Q_ASSERT(false);
			return 0;
		}

		int copyRegCount = regsCount;

		if (regsStartAddr + copyRegCount > m_registers.size())
		{
			copyRegCount = static_cast<int>(m_registers.size()) - regsStartAddr;
		}

		int copyDestSizeBytes = copyRegCount * REGISTER_SIZE_BYTES;

		m_regsMutex.lock(thread);

		std::memcpy(reinterpret_cast<char*>(destBuffer),
					reinterpret_cast<char*>(m_registers.data() + regsStartAddr),
					copyDestSizeBytes);

		m_regsMutex.unlock(thread);

		int fillDestSizeBytes  = (regsCount - copyRegCount) * REGISTER_SIZE_BYTES;

		if (fillDestSizeBytes > 0)
		{
			memset(reinterpret_cast<char*>(destBuffer + copyRegCount), 0, fillDestSizeBytes);
		}

		return regsCount * REGISTER_SIZE_BYTES;
	}

	size_t ModbusSlaveHandler::tcpRequestProcessing(MbshProcData& mpd)
	{
		logTcpRequest(mpd);

		mpd.sendBytes = 0;

		TcpFrame& request = getTcpRequestRef(mpd);

		request.header.reverseBytes();

		Q_ASSERT(request.header.protocolID == 0);

		size_t expectedSize = request.header.length + sizeof(request.header);

		if (expectedSize != mpd.bytesReceived)
		{
			// DEBUG_LOG_ERR(m_listener.log(), QString("ModbusSlaveHandler::onReceiveData error: wrong request size, expected %1, received %2 bytes").
			// 								arg(expectedSize).arg(bytesReceived));
			return 0;
		}

		if (request.msg.modbusDeviceID != modbusDeviceID())
		{
			return 0;					// its Ok, request to another device
		}

		switch(request.msg.functionCode)
		{
		case FC_READ_HOLDING_REGISTERS:
			mpd.sendBytes = onFnReadHoldingRegisters(mpd);
			break;

		default:;
			// DEBUG_LOG_ERR(m_listener.log(), QString("ModbusSlaveHandler::onReceiveData: unknown modbus function code %1. Request ignored.").
			// 								arg(request.functionCode));
		}

		if (mpd.sendBytes > 0)
		{
			logTcpReply(mpd);
		}

		return mpd.sendBytes;
	}

	size_t ModbusSlaveHandler::asciiRequestProcessing(MbshProcData& mpd)
	{
		logAsciiRequest(mpd);

		if (mpd.bytesReceived != ASCII_FN03_REQUEST_SIZE)
		{
			return 0;
		}

		quint8* ptr = mpd.recvBuffer;

		if (*ptr != ASCII_START_MARKER)
		{
			return 0;
		}

		bool result = true;

		ptr += ASCII_START_MARKER_LEN;

		size_t binDataLen = (mpd.bytesReceived - ASCII_START_MARKER_LEN - ASCII_END_MARKER_LEN) / 2;

		for(size_t i = 0; i < binDataLen; i++)
		{
			bool ok = true;

			m_binData[i] = asciiDecodeXX(ptr, &ok);

			if (ok == false)
			{
				return 0;
			}

			ptr += 2;
		}

		Message& msg = *reinterpret_cast<Message*>(m_binData);

		RETURN_IF_FALSE(result);

		if (msg.modbusDeviceID != modbusDeviceID())
		{
			return 0;			// its Ok, request to another device
		}

		if (msg.functionCode != FC_READ_HOLDING_REGISTERS)
		{
			Q_ASSERT(false);
			return false;
		}

		msg.fn03Request.reverseBytes();

		quint16 regsStartAddr = msg.fn03Request.regsStartAddr;
		quint16 regsCount = msg.fn03Request.regsCount;

		quint8 receivedCrc = m_binData[binDataLen - 1];

		RETURN_IF_FALSE(result);

/*		quint8 calculatedCrc = nonStandardModbusCrcCalculation(request + ASCII_START_MARKER_LEN,
																ASCII_DEVICE_ID_LEN +
																ASCII_FUNCTION_LEN +
																ASCII_REG_START_ADDR_LEN +
																ASCII_REG_COUNT_LEN); */

		quint8 calculatedCrc = LRC(m_binData, binDataLen - 1);

		if (receivedCrc != calculatedCrc)
		{
			DEBUG_LOG_ERR(log(), QString("CRC error: received 0x%1, calculated 0x%2").
											arg(receivedCrc, 2, 16, QChar('0')).
											arg(calculatedCrc, 2, 16, QChar('0')));
			//return 0;
		}

		size_t sendBytesCount = onAsciiFnReadHoldingRegisters(regsStartAddr, regsCount, mpd);

		return sendBytesCount;
	}

	size_t ModbusSlaveHandler::rtuRequestProcessing(MbshProcData& mpd)
	{
		Q_ASSERT(false);		// not implemented!

		Q_UNUSED(mpd);
		return 0;
	}

	bool ModbusSlaveHandler::init()
	{
		const std::map<Address16, std::pair<QString, ModbusFormat>>& mbSignals = m_gateway->modbusSignals();

		auto itLast = mbSignals.rbegin();

		if (itLast == mbSignals.rend())
		{
			return true;
		}

		Address16 maxAddr = itLast->first;

		const ModbusFormat lastSignalFormat = itLast->second.second;

		maxAddr.addWord(lastSignalFormat.registersCount());			// maxAddr here is +1 to real registers count, it is ok

		m_regsMutex.lock();

		m_registers.clear();
		m_registers.resize(maxAddr.offset(), 0);

		m_regsMutex.unlock();

		m_signalsStates.clear();

		for(const auto& [addr16, p] : mbSignals)
		{
			const QString& appSignalID = p.first;
			const ModbusFormat& format = p.second;

			Hash hash = calcHash(appSignalID);

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				auto [newIt, b] = m_signalsStates.emplace(hash, std::list<SignalState>{});

				it = newIt;
			}

			it->second.emplace_back(format, addr16);
		}

		return true;
	}

	HostAddressPort ModbusSlaveHandler::listeningIP1() const
	{
		return m_gateway->localGatewayIP1();
	}

	HostAddressPort ModbusSlaveHandler::listeningIP2() const
	{
		return m_gateway->localGatewayIP2();
	}

	void ModbusSlaveHandler::updateAllRegisters()
	{
		m_regsMutex.lock();

		for(const auto& [hash, states] : m_signalsStates)
		{
			for(const SignalState& state : states)
			{
				updateRegister(state);
			}
		}

		m_regsMutex.unlock();
	}

	void ModbusSlaveHandler::updateRegisters(const std::set<Hash>& hashes)
	{
		m_regsMutex.lock();

		for(Hash hash : hashes)
		{
			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			const std::list<SignalState> states = it->second;

			for(const SignalState& state : states)
			{
				updateRegister(state);
			}
		}

		m_regsMutex.unlock();
	}

	void ModbusSlaveHandler::updateRegister(const SignalState& state)
	{
		int regAddr = state.regNo.offset() - 1;		// !!! reagAddr == regNo - 1 !!!

		if (regAddr >= TO_INT(m_registers.size()))
		{
			Q_ASSERT(false);
			return;
		}

		switch(state.format.signalFormat)
		{
		case E::ModbusSignalFormat::DiscreteBit:
			{
				RegisterValue& reg = m_registers[regAddr];

				quint16 mask = 1 << state.regNo.bit();

				mask = reverse16(mask, state.format.byteOrder);

				if (state.value == 0)
				{
					reg &= (~mask);
				}
				else
				{
					reg |= mask;
				}
			}
			break;

		case E::ModbusSignalFormat::AnalogFloat16:
			{
				quint16 f16;

				f16 = encodeFloat16(static_cast<float>(state.value));

				f16 = reverse16(f16, state.format.byteOrder);

				m_registers[regAddr] = f16;
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt16:
			{
				qint16 sint16 = static_cast<qint16>(state.value);
				qint16 uint16 = static_cast<quint16>(sint16);

				uint16 = reverse16(uint16, state.format.byteOrder);

				m_registers[regAddr] = uint16;
			}
			break;

		case E::ModbusSignalFormat::AnalogFloat32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				quint32 uint32 = std::bit_cast<quint32>(static_cast<float>(state.value));

				uint32 = reverse32(uint32, state.format.byteOrder);

				m_registers[regAddr] = static_cast<RegisterValue>(uint32 & 0xFFFF);
				m_registers[regAddr + 1] = static_cast<RegisterValue>(uint32 >> 16);
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				quint32 uint32 = std::bit_cast<quint32>(static_cast<qint32>(state.value));

				uint32 = reverse32(uint32, state.format.byteOrder);

				m_registers[regAddr] = static_cast<RegisterValue>(uint32 & 0xFFFF);
				m_registers[regAddr + 1] = static_cast<RegisterValue>(uint32 >> 16);
			}
			break;

		default:
			Q_ASSERT(false);
		}
	}

	quint16 ModbusSlaveHandler::reverse16(quint16 leValue, E::ModbusByteOrder bo) const
	{
		switch(bo)
		{
		case E::ModbusByteOrder::LE:
		case E::ModbusByteOrder::BE_ByteSwap:
			return leValue;

		case E::ModbusByteOrder::BE:
		case E::ModbusByteOrder::LE_ByteSwap:
			return reverseUint16(leValue);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint32 ModbusSlaveHandler::reverse32(quint32 le_Value, E::ModbusByteOrder bo) const
	{
		switch(bo)
		{
		case E::ModbusByteOrder::LE:
			return le_Value;

		case E::ModbusByteOrder::LE_ByteSwap:
			{
				quint16 leValueLow = static_cast<quint16>(le_Value & 0x0000FFFF);
				quint16 leValueHigh = static_cast<quint16>(le_Value >> 16);

				leValueLow = reverseUint16(leValueLow);
				leValueHigh = reverseUint16(leValueHigh);

				le_Value = leValueHigh;
				le_Value <<= 16;
				le_Value |= leValueLow;

				return le_Value;
			}

		case E::ModbusByteOrder::BE_ByteSwap:
			{
				// swap low and high 16 bit words only!
				//
				quint16 leValueHigh = static_cast<quint16>(le_Value >> 16);

				le_Value <<= 16;
				le_Value |= leValueHigh;

				return le_Value;
			}

		case E::ModbusByteOrder::BE:
			return reverseUint32(le_Value);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	void ModbusSlaveHandler::logTcpRequest(MbshProcData& mpd)
	{
		if (enableLogging() == false)
		{
			return;
		}

		m_logStr.clear();

		m_logStr.append(QString("Gateway %1 #%2 Modbus %3 request from %4, socket error code %5 ('%6'), bytes received %7").
						arg(gatewayID()).
						arg(mpd.connNo).
						arg(::E::valueToString(modbusMode())).
						arg(mpd.peerAddr).
						arg(mpd.error.value()).
						arg(mpd.error ? QString::fromStdString(mpd.error.message()) : QStringLiteral("NoErr")).
						arg(mpd.bytesReceived));

		logRequest(m_logStr);

		if (mpd.error || mpd.bytesReceived == 0)
		{
			return;
		}

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Binary:"));

		for(size_t i = 0; i < mpd.bytesReceived && i < mpd.recvBufferSize; i++)
		{
			m_logStr.append(QString(" %1").arg(mpd.recvBuffer[i], 2, 16, QChar('0')).toUpper());
		}

		logRequest(m_logStr);

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("TCP header: "));

		const TcpFrame& req = getTcpRequestRef(mpd);

		TcpHeader reqHeader = req.header;

		reqHeader.reverseBytes();

		m_logStr.append(QString("transactionID %1, protocolID %2, length %3").
						arg(reqHeader.transactionID).arg(reqHeader.protocolID).arg(reqHeader.length));

		logRequest(m_logStr);

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Modbus request: "));

		switch(req.msg.functionCode)
		{
		case FC_READ_HOLDING_REGISTERS:
		{
			Fn03_ReadHoldingRegisters_Request fn03Req = req.msg.fn03Request;

			fn03Req.reverseBytes();

			m_logStr.append(QString("deviceID %1, function %2, start register %3, regs count %4").
							arg(req.msg.modbusDeviceID).
							arg(req.msg.functionCode).
							arg(fn03Req.regsStartAddr).
							arg(fn03Req.regsCount));
			logRequest(m_logStr);
		}
		break;

		default:
			m_logStr.append(QString("function %1 is not supported!").arg(req.msg.functionCode));
			logRequest(m_logStr, CircularLogger::RecordType::Error);
		}
	}

	void ModbusSlaveHandler::logTcpReply(MbshProcData& mpd)
	{
		if (enableLogging() == false)
		{
			return;
		}

		m_logStr.clear();

		m_logStr.append(QString("Gateway %1 #%2 Modbus TCP reply to %3, bytes sent %4").
						arg(gatewayID()).
						arg(mpd.connNo).
						arg(mpd.peerAddr).
						arg(mpd.sendBytes));

		logReply(m_logStr);

		if (mpd.sendBytes == 0)
		{
			return;
		}

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Binary:"));

		for(size_t i = 0; i < mpd.sendBytes && i < mpd.sendBufferSize; i++)
		{
			m_logStr.append(QString(" %1").arg(mpd.sendBuffer[i], 2, 16, QChar('0')).toUpper());
		}

		logReply(m_logStr);

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("TCP header: "));

		const TcpFrame& rep = getTcpReplyRef(mpd);

		TcpHeader repHeader = rep.header;

		repHeader.reverseBytes();

		m_logStr.append(QString("transactionID %1, protocolID %2, length %3").
						arg(repHeader.transactionID).arg(repHeader.protocolID).arg(repHeader.length));

		logReply(m_logStr);

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Modbus reply: "));

		switch(rep.msg.functionCode)
		{
		case FC_READ_HOLDING_REGISTERS:
		{
			const Fn03_ReadHoldingRegisters_Reply& fn03Rep = rep.msg.fn03Reply;

			quint8 bytesCount = rep.msg.fn03Reply.bytesCount;
			quint32 regsCount = static_cast<quint32>(bytesCount / sizeof(quint16));

			m_logStr.append(QString("deviceID %1, function %2, bytes count %3, regs count %4").
							arg(rep.msg.modbusDeviceID).
							arg(rep.msg.functionCode).
							arg(bytesCount).
							arg(regsCount));
			logReply(m_logStr);

			//

			m_logStr.clear();

			m_logStr.append(QStringLiteral("Modbus reply:"));

			const TcpFrame& req = getTcpRequestRef(mpd);

			for(quint32 i = 0; i < regsCount; i++)
			{
				m_logStr.append(QString(" r[%1]=0x%3").
								arg(req.msg.fn03Request.regsStartAddr + i).
								arg(fn03Rep.regValues[i], 4, 16, QChar('0')));
			}
			logReply(m_logStr);
		}
		break;

		default:
			m_logStr.append(QString("function %1 is not supported!").arg(rep.msg.functionCode));
			logReply(m_logStr, CircularLogger::RecordType::Error);
		}
	}

	void ModbusSlaveHandler::logAsciiRequest(MbshProcData& mpd)
	{
		if (enableLogging() == false)
		{
			return;
		}

		m_logStr.clear();

		m_logStr.append(QString("Gateway %1 #%2 Modbus %3 request from %4, socket error code %5 ('%6'), bytes received %7").
						arg(gatewayID()).
						arg(mpd.connNo).
						arg(::E::valueToString(modbusMode())).
						arg(mpd.peerAddr).
						arg(mpd.error.value()).
						arg(mpd.error ? QString::fromStdString(mpd.error.message()) : QStringLiteral("NoErr")).
						arg(mpd.bytesReceived));

		logRequest(m_logStr);

		//

		if (mpd.error || mpd.bytesReceived == 0)
		{
			return;
		}

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("ASCII:  "));
		m_logStr.append(mpd.recvBuffer, mpd.bytesReceived);

		m_logStr.replace(Separator::CR, "\\r");
		m_logStr.replace(Separator::LF, "\\n");

		logRequest(m_logStr);

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Binary:"));

		for(size_t i = 0; i < mpd.bytesReceived && i < mpd.recvBufferSize; i++)
		{
			m_logStr.append(QString(" %1").arg(mpd.recvBuffer[i], 2, 16, QChar('0')).toUpper());
		}

		logRequest(m_logStr);

		//

		if (mpd.bytesReceived != ASCII_FN03_REQUEST_SIZE)
		{
			logRequest(QString("Wrong Modbus ASCII F03 request len %1, expected %2. Request ignored.").
					   arg(mpd.bytesReceived).arg(ASCII_FN03_REQUEST_SIZE));
			return 0;
		}

		//

		m_logStr.clear();

		m_logStr.append(QStringLiteral("Modbus request: "));

		switch(req.msg.functionCode)
		{
		case FC_READ_HOLDING_REGISTERS:
		{
			Fn03_ReadHoldingRegisters_Request fn03Req = req.msg.fn03Request;

			fn03Req.reverseBytes();

			m_logStr.append(QString("deviceID %1, function %2, start register %3, regs count %4").
							arg(req.msg.modbusDeviceID).
							arg(req.msg.functionCode).
							arg(fn03Req.regsStartAddr).
							arg(fn03Req.regsCount));
			logRequest(m_logStr);
		}
		break;

		default:
			m_logStr.append(QString("function %1 is not supported!").arg(req.msg.functionCode));
			logRequest(m_logStr, CircularLogger::RecordType::Error);
		}
	}

	void ModbusSlaveHandler::logAsciiReply(MbshProcData& mpd)
	{
		if (enableLogging() == false)
		{
			return;
		}

	}

	void ModbusSlaveHandler::logRtuRequest(MbshProcData& mpd)
	{
		Q_UNUSED(mpd);
	}

	void ModbusSlaveHandler::logRtuReply(MbshProcData& mpd)
	{
		Q_UNUSED(mpd);
	}

	size_t ModbusSlaveHandler::onFnReadHoldingRegisters(MbshProcData& mpd)
	{
		TcpFrame& request = getTcpRequestRef(mpd);

		if (request.msg.functionCode != FC_READ_HOLDING_REGISTERS)
		{
			Q_ASSERT(false);
			return 0;
		}

		Fn03_ReadHoldingRegisters_Request& fn03Request = request.msg.fn03Request;

		fn03Request.reverseBytes();

		int regsStartAddr = fn03Request.regsStartAddr;
		int regsCount = fn03Request.regsCount;

		Q_ASSERT(regsCount <= 127);

		TcpFrame& reply = getTcpReplyRef(mpd);

		int bytesCount = getRegistersValues(regsStartAddr, regsCount,
											reply.msg.fn03Reply.regValues, FN03_MAX_REGS_COUNT,
											QThread::currentThread());
		Q_ASSERT(bytesCount < 256);

		// copy request header fields to reply
		//
		reply.header.transactionID = request.header.transactionID;
		reply.header.protocolID = request.header.protocolID;

		// set size of reply data after header
		//
		reply.header.length = sizeof(reply.msg.modbusDeviceID) +
							  sizeof(reply.msg.functionCode) +
							  sizeof(reply.msg.fn03Reply.bytesCount) +
							  bytesCount;

		// copy request function params to reply
		//
		reply.msg.modbusDeviceID = request.msg.modbusDeviceID;
		reply.msg.functionCode = request.msg.functionCode;

		// fill reply bytes count
		//
		reply.msg.fn03Reply.bytesCount = static_cast<quint8>(bytesCount);

		//

		size_t sendBytesCount = sizeof(reply.header) + reply.header.length;

		reply.header.reverseBytes();		// translate header fields to BE

		return sendBytesCount;
	}

	TcpFrame& ModbusSlaveHandler::getTcpRequestRef(MbshProcData& mpd)
	{
		Q_ASSERT(mpd.recvBuffer != nullptr);
		return *reinterpret_cast<TcpFrame*>(mpd.recvBuffer);
	}

	TcpFrame& ModbusSlaveHandler::getTcpReplyRef(MbshProcData& mpd)
	{
		Q_ASSERT(mpd.sendBuffer != nullptr);
		return *reinterpret_cast<TcpFrame*>(mpd.sendBuffer);
	}

	size_t ModbusSlaveHandler::onAsciiFnReadHoldingRegisters(quint16 regsStartAddr, quint16 regsCount,
															 MbshProcData& mpd)
	{
		if (regsCount > FN03_MAX_REGS_COUNT)
		{
			return 0;
		}

		Message& msg = *reinterpret_cast<Message*>(m_binData);

		size_t binDataLen = 0;

		binDataLen += sizeof(msg.modbusDeviceID);
		binDataLen += sizeof(msg.functionCode);

		int bytesCount = getRegistersValues(regsStartAddr, regsCount,
											msg.fn03Reply.regValues, FN03_MAX_REGS_COUNT,
											QThread::currentThread());

		Q_ASSERT(bytesCount == regsCount * sizeof(RegisterValue));
		Q_ASSERT(bytesCount <= 0xFF);

		msg.fn03Reply.bytesCount = static_cast<quint8>(bytesCount);

		binDataLen += sizeof(msg.fn03Reply.bytesCount);
		binDataLen += msg.fn03Reply.bytesCount;

		// quint8 crc = nonStandardModbusCrcCalculation(m_binData,	binDataLen);		// 2 chars on 1 byte

		quint8 crc = LRC(m_binData,	binDataLen);		// 2 chars on 1 byte

		m_binData[binDataLen] = crc;

		binDataLen += sizeof(crc);

		// Encoding bin data to ASCII
		//
		quint8* ptr = mpd.sendBuffer;

		*ptr = ':';
		ptr++;

		for(size_t i = 0; i < binDataLen; i++)
		{
			ptr = asciiEncodeXX(m_binData[i], ptr);
		}

		*ptr = ASCII_END_MARKER_1;
		ptr++;

		*ptr = ASCII_END_MARKER_2;
		ptr++;

		size_t sendBytes = ptr - mpd.sendBuffer;

		mpd.sendBytes = sendBytes;

		return sendBytes;
	}

	bool ModbusSlaveHandler::convertAsciiToBin(quint8* asciiPtr, size_t asciiLen,
											quint8* binPtr, size_t* binLen)
	{
		TEST_PTR_RETURN_FALSE(asciiPtr);
		TEST_PTR_RETURN_FALSE(binPtr);
		TEST_PTR_RETURN_FALSE(binLen);
	}

	bool ModbusSlaveHandler::isHexDigits(const quint8* ptr, int len) const
	{
		int result = 1;

		for(int i = 0; i < len; i++)
		{
			result &= std::isxdigit(ptr[i]);
		}

		return (result == 0 ? false : true);
	}

	quint8 ModbusSlaveHandler::asciiDecodeXX(const quint8* ptr, bool* ok) const
	{
		quint64 result = asciiDecode(ptr, sizeof(quint8) * 2, ok);
		Q_ASSERT(result <= 0xFF);
		return static_cast<quint8>(result);
	}

	quint16 ModbusSlaveHandler::asciiDecodeXXXX(const quint8* ptr, bool* ok) const
	{
		quint64 result = asciiDecode(ptr, sizeof(quint16) * 2, ok);
		Q_ASSERT(result <= 0xFFFF);
		return static_cast<quint16>(result);
	}

	quint64 ModbusSlaveHandler::asciiDecode(const quint8* ptr, int len, bool* ok) const
	{
		TEST_PTR_RETURN_VALUE(ptr, 0);
		TEST_PTR_RETURN_VALUE(ok, 0);

		if(len <= 0 || len > sizeof(quint64) * 2)
		{
			Q_ASSERT(false);
			*ok = false;
			return 0;
		}

		quint64 result = 0;

		for(int i = 0; i < len; i++)
		{
			unsigned char ch = ptr[i];

			ch = asciiDecodeX(ch, ok);

			if (*ok == false)
			{
				return 0;
			}

			result <<= 4;
			result += ch;
		}

		*ok = true;

		return result;
	}

	quint8 ModbusSlaveHandler::asciiDecodeX(quint8 ch, bool* ok) const
	{
		if (ch >= '0' && ch <= '9')
		{
			ch -= '0';
		}
		else
		{
			if (ch >= 'A' && ch <= 'F')
			{
				ch = 0x0A + ch - 'A';
			}
			else
			{
				if (ch >= 'a' && ch <= 'f')
				{
					ch = 0x0A + ch - 'a';
				}
				else
				{
					Q_ASSERT(false);		// ch is not a hex digit!
					*ok = false;
					return 0;
				}
			}
		}

		return ch;
	}

	quint8* ModbusSlaveHandler::asciiEncodeXX(quint8 v8, quint8* ptr)
	{
		*ptr = asciiEncodeX((v8 >> 4) & 0x0F);
		ptr++;

		*ptr = asciiEncodeX(v8 & 0x0F);
		ptr++;

		return ptr;
	}

	quint8* ModbusSlaveHandler::asciiEncodeXXXX(quint16 v16, quint8* ptr)
	{
		ptr = asciiEncodeXX((v16 >> 8) & 0xFF, ptr);
		ptr = asciiEncodeXX(v16 & 0xFF, ptr);

		return ptr;
	}

	quint8 ModbusSlaveHandler::asciiEncodeX(quint8 ch)
	{
		if (ch <= 9)
		{
			return ch + '0';
		}

		if (ch <= 0x0F)
		{
			return ch - 0x0A + 'A';
		}

		Q_ASSERT(false);

		return '0';
	}

	quint8 ModbusSlaveHandler::nonStandardModbusCrcCalculation(const quint8* ptr, int lenInChars)
	{
		// Non-standart modbus request CRC calculation used on AEC Kozloduy in UIK system.
		//
		// Reverse ingeneered from request making code:
		//
		// static void makeRequest(array<unsigned char>^ request, int chan, bool Ust)
		// {
		// 	request[0] = ':'; //header
		// 	request[1] = '0'; //slave address
		// 	request[2] = '0' + chan;
		// 	request[3] = '0'; //function
		// 	request[4] = '3';
		// 	request[5] = '0'; //start address Hi
		// 	request[6] = '0';
		// 	request[7] = '0'; //start Lo
		// 	request[8] = '0';
		// 	request[9] = '0'; //Number Hi
		// 	request[10]= '0';
		// 	request[11]= '6'; //Number Lo
		// 	request[12]= '6';
		// 	request[13]= '0'; //CRC
		// 	request[14]= '0';
		// 	request[15]= 0x0D;
		// 	request[16]= 0x0A;
		//
		// 	if(Ust)
		// 	{
		// 		request[7] = '6'; //start Lo
		// 		request[8] = '6';
		// 		request[11]= '4'; //Number Lo
		// 		request[12]= '6';
		// 	}
		//
		// ---------------- Non-standard CRC calculation! -----------------------
		//
		//	1. ASCII decoding is used before summing CRC
		//  2. Result is not two's complementing
		//
		//	Standard Modbus LRC calculation see in ModbusProtocol.cpp
		//
		//  //
		//
		// 	unsigned int CRC=0;
		// 	for(int i=1;i<13;i+=2)
		// 		CRC+=uncodeASCII(request[i])*16 + uncodeASCII(request[i+1]);
		// 	CRC = CRC & 0xFF;
		// ----------------------------------------------------------------------
		//
		// 	request[13] = codeASCII(CRC>>4);
		// 	request[14] = codeASCII(CRC&0xF);
		// }

		quint8 crc = 0;
		bool ok = true;

		for(int i = 0; i < lenInChars; i++)
		{
			quint8 ch = asciiDecodeX(ptr[0], &ok);

			Q_ASSERT(ok == true);

			crc += ch;
		}

		return crc;
	}
}
