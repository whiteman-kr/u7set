#include "TuningSocket.h"

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSocket class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort, TuningBase* pTuningBase)
        : Tcp::Client(softwareInfo, serverAddressPort, "TuningSocket")
	, m_pTuningBase(pTuningBase)
{
}

TuningSocket::~TuningSocket()
{
}

void TuningSocket::onClientThreadStarted()
{
	//std::cout << "TuningSocket::onClientThreadStarted()\n";
}

void TuningSocket::onClientThreadFinished()
{
	//std::cout << "TuningSocket::onClientThreadFinished()\n";
}

void TuningSocket::onConnection()
{
	//std::cout << "TuningSocket::onConnection()\n";

	emit socketConnected();

	requestTuningSourcesInfo();
}

void TuningSocket::onDisconnection()
{
	//std::cout << "TuningSocket::onDisconnection\n";

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

		case TDS_TUNING_SIGNALS_READ:
			replyReadTuningSignals(replyData, replyDataSize);
			break;

		case TDS_TUNING_SIGNALS_WRITE:
			replyWriteTuningSignals(replyData, replyDataSize);
			break;

		default:
			assert(false);
	}
}

// TDS_GET_TUNING_SOURCES_INFO

void TuningSocket::requestTuningSourcesInfo()
{
	assert(isClearToSendRequest());

	if (m_pTuningBase != nullptr)
	{
		m_pTuningBase->Sources().clear();
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
		std::cout << "TuningSocket::replyTuningSourcesInfo - error: ParseFromArray\n";
		assert(result);
		requestTuningSourcesInfo();
		return;
	}

	if (m_tuningDataSourcesInfoReply.error() != 0)
	{
		std::cout << "TuningSocket::replyTuningSourcesInfo - error: " << m_tuningDataSourcesInfoReply.error() << "\n";
		assert(m_tuningDataSourcesInfoReply.error() != 0);
		requestTuningSourcesInfo();
		return;
	}

	if (m_pTuningBase == nullptr)
	{
		std::cout << "TuningSocket::replyTuningSourcesInfo - error: failed TuningBase\n";
		assert(false);
		requestTuningSourcesInfo();
		return;
	}

	int sourceCount = m_tuningDataSourcesInfoReply.datasourceinfo_size();
	if (sourceCount == 0)
	{
		std::cout << "Error : Tuning sources count: " << sourceCount << "\n";
	}

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::DataSourceInfo& dsi = m_tuningDataSourcesInfoReply.datasourceinfo(i);
		m_pTuningBase->Sources().append(TuningSource(dsi));

		//std::cout << "TuningSocket::replyTuningSourcesInfo - : " << i << ". SubSystem:" << dsi.lmsubsystem().c_str() << ", EquipmentID:" << dsi.lmequipmentid().c_str() << ", IP:" << dsi.lmip().c_str() << "\n";
		std::cout << "TuningSource:" << dsi.lmequipmentid().c_str() << ", IP:" << dsi.lmip().c_str() << "\n";
	}

	m_pTuningBase->Sources().sortByID();

	emit sourcesLoaded();

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
		std::cout << "TuningSocket::replyTuningSourcesState - error: ParseFromArray\n";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_tuningDataSourcesStatesReply.error() != 0)
	{
		std::cout << "TuningSocket::replyTuningSourcesState - error: " << m_tuningDataSourcesStatesReply.error() << "\n";
		assert(m_tuningDataSourcesStatesReply.error() != 0);
		requestTuningSourcesState();
		return;
	}

	if (m_pTuningBase == nullptr)
	{
		std::cout << "TuningSocket::replyTuningSourcesState - error: failed TuningBase\n";
		assert(false);
		requestTuningSourcesState();
		return;
	}

	int sourceCount = m_tuningDataSourcesStatesReply.tuningsourcesstate_size();

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningsourcesstate(i);
		m_pTuningBase->Sources().setState(tss.sourceid(), tss);
	}

	requestReadTuningSignals();
}

// TDS_TUNING_SIGNALS_READ

void TuningSocket::requestReadTuningSignals()
{
	assert(isClearToSendRequest());

	if (m_pTuningBase == nullptr)
	{
		std::cout << "TuningSocket::requestReadTuningSignals - error: failed TuningBase\n";
		assert(false);
		requestTuningSourcesState();
		return;
	}

	int signalForReadCount = m_pTuningBase->Signals().count();
	if (signalForReadCount == 0)
	{
		requestWriteTuningSignals();
		return;
	}

	m_readTuningSignals.mutable_signalhash()->Reserve(signalForReadCount);

	m_readTuningSignals.mutable_signalhash()->Clear();

	int startIndex = m_readTuningSignalsIndex;

	for (int i = 0; i < TUNING_SOCKET_MAX_READ_SIGNAL; i++)
	{
		if (m_readTuningSignalsIndex >= signalForReadCount)
		{
			m_readTuningSignalsIndex = 0;
			break;
		}

		TestSignal* pSignal = m_pTuningBase->Signals().signal(i + startIndex);
		if (pSignal == nullptr)
		{
			continue;
		}

		Signal& param = pSignal->param();
		if (param.appSignalID().isEmpty() == true || param.hash() == 0)
		{
			continue;
		}

		m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(param.hash());

		m_readTuningSignalsIndex ++;
	}

	sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);
}

void TuningSocket::replyReadTuningSignals(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_readTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), static_cast<int>(replyDataSize));
	if (result == false)
	{
		std::cout << "TuningSocket::replyReadTuningSignals - error: ParseFromArray\n";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_readTuningSignalsReply.error() != 0)
	{
		std::cout << "TuningSocket::replyReadTuningSignals - error: " << m_readTuningSignalsReply.error() << "\n";
		assert(m_readTuningSignalsReply.error() != 0);
		requestTuningSourcesState();
		return;
	}

	if (m_pTuningBase == nullptr)
	{
		std::cout << "TuningSocket::replyReadTuning - error: failed TuningBase\n";
		assert(false);
		requestTuningSourcesState();
		return;
	}

	int readReplyCount = m_readTuningSignalsReply.tuningsignalstate_size();

	for (int i = 0; i < readReplyCount; i++)
	{
		m_pTuningBase->Signals().setState(m_readTuningSignalsReply.tuningsignalstate(i));
	}

	requestWriteTuningSignals();
}

// TDS_TUNING_SIGNALS_WRITE

void TuningSocket::requestWriteTuningSignals()
{
	assert(isClearToSendRequest());

	if (m_pTuningBase == nullptr)
	{
		std::cout << "TuningSocket::requestWriteTuningSignals - error: failed TuningBase\n";
		assert(false);
		requestTuningSourcesState();
		return;
	}


	int cmdCount = m_pTuningBase->cmdFowWriteCount();
	if (cmdCount == 0)
	{
		requestTuningSourcesState();
		return;
	}

	m_writeTuningSignals.set_autoapply(true);

	m_writeTuningSignals.mutable_commands()->Reserve(cmdCount);

	m_writeTuningSignals.mutable_commands()->Clear();

	for (int i = 0; i < cmdCount && i < TUNING_SOCKET_MAX_WRITE_CMD; i++)
	{
		TuningWriteCmd cmd = m_pTuningBase->cmdFowWrite();

		if (cmd.signalHash() == 0)
		{
			assert(cmd.signalHash() != 0);
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

void TuningSocket::replyWriteTuningSignals(const char* replyData, quint32 replyDataSize)
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
		//std::cout << "TuningSocket::replyWriteTuningSignals - error: ParseFromArray\n";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_writeTuningSignalsReply.error() != 0)
	{
		//std::cout << "TuningSocket::replyWriteTuningSignals - error: " << m_writeTuningSignalsReply.error() + "\n";
		requestTuningSourcesState();
		return;
	}

	requestTuningSourcesState();
}
