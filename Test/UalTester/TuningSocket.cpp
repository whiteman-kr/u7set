#include "TuningSocket.h"

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSocket class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort, TuningSourceBase* pTuningSourceBase)
	: Tcp::Client(softwareInfo, serverAddressPort, "TuningSocket")
	, m_pTuningSourceBase(pTuningSourceBase)
{
}

TuningSocket::~TuningSocket()
{
}

void TuningSocket::onClientThreadStarted()
{
	//std::cout << "TuningSocket::onClientThreadStarted()" << std::endl;
}

void TuningSocket::onClientThreadFinished()
{
	//std::cout << "TuningSocket::onClientThreadFinished()" << std::endl;
}

void TuningSocket::onConnection()
{
	//std::cout << "TuningSocket::onConnection()" << std::endl;

	emit socketConnected();

	requestTuningSourcesInfo();
}

void TuningSocket::onDisconnection()
{
	//std::cout << "TuningSocket::onDisconnection" << std::endl;

	emit socketDisconnected();
}

void TuningSocket::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	switch(requestID)
	{
		case TDS_GET_TUNING_SOURCES_INFO:
			replyTuningSourcesInfo(replyData, replyDataSize);
			break;

		case TDS_GET_TUNING_SOURCES_STATES:
			replyTuningSourcesState(replyData, replyDataSize);
			break;

		case TDS_TUNING_SIGNALS_WRITE:
			replyWriteStateTuningSignals(replyData, replyDataSize);
			break;

		case TDS_DATA_SOURCE_WRITE:
			replyWriteStateDataSource(replyData, replyDataSize);
			break;

		case TDS_PACKETSOURCE_EXIT:
			replyPacketSourceExit(replyData, replyDataSize);
			break;

		default:
			assert(false);
	}
}

// TDS_GET_TUNING_SOURCES_INFO

void TuningSocket::requestTuningSourcesInfo()
{
	assert(isClearToSendRequest());

	if (m_pTuningSourceBase != nullptr)
	{
		m_pTuningSourceBase->clear();
	}

	sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

void TuningSocket::replyTuningSourcesInfo(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesInfo();
		return;
	}

	bool result = m_tuningDataSourcesInfoReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		std::cout << __FUNCTION__ << " - error: ParseFromArray" << std::endl;
		assert(result);
		requestTuningSourcesInfo();
		return;
	}

	if (m_tuningDataSourcesInfoReply.error() != 0)
	{
		std::cout << __FUNCTION__ << " - error: " << m_tuningDataSourcesInfoReply.error() << std::endl;
		assert(m_tuningDataSourcesInfoReply.error() != 0);
		requestTuningSourcesInfo();
		return;
	}

	if (m_pTuningSourceBase == nullptr)
	{
		std::cout << __FUNCTION__ << " - error: failed SourceBase" << std::endl;
		assert(false);
		requestTuningSourcesInfo();
		return;
	}

	int sourceCount = m_tuningDataSourcesInfoReply.datasourceinfo_size();
	if (sourceCount == 0)
	{
		std::cout << "Error : Tuning sources count: " << sourceCount << std::endl;
	}

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::DataSourceInfo& dsi = m_tuningDataSourcesInfoReply.datasourceinfo(i);
		m_pTuningSourceBase->append(TuningSource(dsi));

		// std::cout << __FUNCTION__ << " - : " << i << ". SubSystem:" << dsi.lmsubsystem().c_str() << ", EquipmentID:" << dsi.lmequipmentid().c_str() << ", IP:" << dsi.lmip().c_str() << std::endl;
	}

	m_pTuningSourceBase->sortByID();

	requestTuningSourcesState();
}

// TDS_GET_TUNING_SOURCES_STATES

void TuningSocket::requestTuningSourcesState()
{
	assert(isClearToSendRequest());

	QThread::msleep(TUNING_SOCKET_TIMEOUT_STATE);

	sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

void TuningSocket::replyTuningSourcesState(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_tuningDataSourcesStatesReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		std::cout << __FUNCTION__ << " - error: ParseFromArray" << std::endl;
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_tuningDataSourcesStatesReply.error() != 0)
	{
		std::cout << __FUNCTION__ << " - error: " << m_tuningDataSourcesStatesReply.error() << std::endl;
		assert(m_tuningDataSourcesStatesReply.error() != 0);
		requestTuningSourcesState();
		return;
	}

	if (m_pTuningSourceBase == nullptr)
	{
		std::cout << __FUNCTION__ << " - error: failed SourceBase" << std::endl;
		assert(false);
		requestTuningSourcesState();
		return;
	}

	int sourceCount = m_tuningDataSourcesStatesReply.tuningsourcesstate_size();

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningsourcesstate(i);
		m_pTuningSourceBase->setState(static_cast<qint64>(tss.sourceid()), tss);
	}

	requestWriteStateTuningSignals();
}


// TDS_TUNING_SIGNALS_WRITE

void TuningSocket::requestWriteStateTuningSignals()
{
	assert(isClearToSendRequest());

	int cmdCount = cmdSignalStateCount();
	if (cmdCount == 0)
	{
		requestWriteStateDataSource();
		return;
	}

	m_writeTuningSignals.set_autoapply(true);

	m_writeTuningSignals.mutable_commands()->Reserve(cmdCount);

	m_writeTuningSignals.mutable_commands()->Clear();

	for (int i = 0; i < cmdCount && i < TUNING_SOCKET_MAX_WRITE_CMD; i++)
	{
		ChangeSignalState cmd = cmdSignalState();

		if (cmd.signalHash() == UNDEFINED_HASH)
		{
			assert(cmd.signalHash() != UNDEFINED_HASH);
			continue;
		}

		Network::TuningWriteCommand* wrCmd = new Network::TuningWriteCommand();
		if (wrCmd == nullptr)
		{
			continue;
		}

		wrCmd->set_signalhash(cmd.signalHash());

		Proto::TuningValue* tv = new Proto::TuningValue();

		tv->set_type(cmd.typeInt());

		switch (cmd.type())
		{
			case TuningValueType::Discrete:
			case TuningValueType::SignedInt32:	tv->set_intvalue(cmd.value().toInt());			break;
			case TuningValueType::SignedInt64:	tv->set_intvalue(cmd.value().toLongLong());		break;
			case TuningValueType::Float:
			case TuningValueType::Double:		tv->set_doublevalue(cmd.value().toDouble());	break;
			default:							assert(false); continue;						break;
		}

		wrCmd->set_allocated_value(tv);

		m_writeTuningSignals.mutable_commands()->AddAllocated(wrCmd);
	}

	sendRequest(TDS_TUNING_SIGNALS_WRITE, m_writeTuningSignals);
}

void TuningSocket::replyWriteStateTuningSignals(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_writeTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		//std::cout << __FUNCTION__ << " - error: ParseFromArray" << std::endl;
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_writeTuningSignalsReply.error() != 0)
	{
		//std::cout << __FUNCTION__ << " - error: " << m_writeTuningSignalsReply.error() << std::endl;
		requestTuningSourcesState();
		return;
	}

	requestWriteStateDataSource();
}

// TDS_DATA_SOURCE_WRITE

void TuningSocket::requestWriteStateDataSource()
{
	assert(isClearToSendRequest());

	int cmdCount = cmdSourceStateCount();
	if (cmdCount == 0)
	{
		requestPacketSourceExit();
		return;
	}

	ChangeSourceState cmd = cmdSourceState();

	if (cmd.sourceID().isEmpty() == true)
	{
		requestTuningSourcesState();
		return;
	}

	m_dataSourceWrite.set_sourceequipmentid(cmd.sourceID().toStdString());
	m_dataSourceWrite.set_state(cmd.state());

	sendRequest(TDS_DATA_SOURCE_WRITE, m_dataSourceWrite);
}

void TuningSocket::replyWriteStateDataSource(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_dataSourceWriteReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		//std::cout << __FUNCTION__ << " - error: ParseFromArray" << std::endl;
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_dataSourceWriteReply.error() != 0)
	{
		//std::cout << __FUNCTION__ << " - error: " << m_psSourceWriteReply.error() << std::endl;
		requestTuningSourcesState();
		return;
	}

	requestPacketSourceExit();
}

// TDS_PACKETSOURCE_EXIT

void TuningSocket::requestPacketSourceExit()
{
	assert(isClearToSendRequest());

	if (m_cmdExitPacketSource == false)
	{
		requestTuningSourcesState();
		return;
	}

	writeCmd(false);

	sendRequest(TDS_PACKETSOURCE_EXIT, m_packetSourceExit);
}

void TuningSocket::replyPacketSourceExit(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_packetSourceExitReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		//std::cout << __FUNCTION__ << " - error: ParseFromArray" << std::endl;
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_packetSourceExitReply.error() != 0)
	{
		//std::cout << __FUNCTION__ << " - error: " << m_tuningExitPsReply.error() << std::endl;
		requestTuningSourcesState();
		return;
	}

	requestTuningSourcesState();
}


// Commands for change source state
//
int TuningSocket::cmdSourceStateCount() const
{
	int count = 0;

	m_sourceStateMutex.lock();

		count = m_cmdSourceStateList.count();

	m_sourceStateMutex.unlock();

	return count;
}

void TuningSocket::writeCmd(const ChangeSourceState& cmd)
{
	if (cmd.sourceID().isEmpty() == true)
	{
		assert(0);
		return;
	}

	m_sourceStateMutex.lock();

		m_cmdSourceStateList.append(cmd);

	m_sourceStateMutex.unlock();
}

ChangeSourceState TuningSocket::cmdSourceState()
{
	ChangeSourceState cmd;

	m_sourceStateMutex.lock();

		if (m_cmdSourceStateList.isEmpty() == false)
		{
			cmd = m_cmdSourceStateList[0];

			m_cmdSourceStateList.remove(0);
		}

	m_sourceStateMutex.unlock();

	return cmd;
}

// Commands for change signal state
//
int TuningSocket::cmdSignalStateCount() const
{
	int count = 0;

	m_signalStateMutex.lock();

		count = m_cmdSignalStateList.count();

	m_signalStateMutex.unlock();

	return count;
}

void TuningSocket::writeCmd(const ChangeSignalState& cmd)
{
	if (cmd.signalHash() == UNDEFINED_HASH)
	{
		assert(cmd.signalHash() != 0);
		return;
	}

	m_signalStateMutex.lock();

		m_cmdSignalStateList.append(cmd);

	m_signalStateMutex.unlock();
}

ChangeSignalState TuningSocket::cmdSignalState()
{
	ChangeSignalState cmd;

	m_signalStateMutex.lock();

		if (m_cmdSignalStateList.isEmpty() == false)
		{
			cmd = m_cmdSignalStateList[0];

			m_cmdSignalStateList.remove(0);
		}

	m_signalStateMutex.unlock();

	return cmd;
}

void TuningSocket::writeCmd(bool exitPacketSource)
{
	m_cmdExitPacketSource = exitPacketSource;
}
