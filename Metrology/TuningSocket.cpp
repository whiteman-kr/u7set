#include "TuningSocket.h"

#include <assert.h>

#include "TuningSignalBase.h"
#include "Options.h"


// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSocket::~TuningSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onClientThreadStarted()
{
	qDebug() << "TuningSocket::onClientThreadStarted()";
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onClientThreadFinished()
{
	qDebug() << "TuningSocket::onClientThreadFinished()";
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onConnection()
{
	qDebug() << "TuningSocket::onConnection()";

	emit socketConnected();

	requestTuningSourcesInfo();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onDisconnection()
{
	qDebug() << "TuningSocket::onDisconnection";

	emit socketDisconnected();
}

// -------------------------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------------------------
// TDS_GET_TUNING_SOURCES_INFO

void TuningSocket::requestTuningSourcesInfo()
{
	assert(isClearToSendRequest());

	theSignalBase.tuning().Sources().clear();

	int serverType = selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return;
	}

	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_TUNING).equipmentID(serverType);
	if (equipmentID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyTuningSourcesInfo(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesInfo();
		return;
	}

	bool result = m_tuningDataSourcesInfoReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
	if (result == false)
	{
		qDebug() << "TuningSocket::replyTuningSourcesInfo - error: ParseFromArray";
		assert(result);
		requestTuningSourcesInfo();
		return;
	}

	if (m_tuningDataSourcesInfoReply.error() != 0)
	{
		qDebug() << "TuningSocket::replyTuningSourcesInfo - error: " << m_tuningDataSourcesInfoReply.error();
		assert(m_tuningDataSourcesInfoReply.error() != 0);
		requestTuningSourcesInfo();
		return;
	}

	int sourceCount = m_tuningDataSourcesInfoReply.datasourceinfo_size();

	qDebug() << "TuningSocket::replyTuningSourcesInfo - Tuning sources count: " << sourceCount;

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::DataSourceInfo& dsi = m_tuningDataSourcesInfoReply.datasourceinfo(i);
		theSignalBase.tuning().Sources().append(TuningSource(dsi));

		qDebug() << "TuningSocket::replyTuningSourcesInfo - : " << i << ". SubSystem:" << dsi.lmsubsystem().c_str() << ", EquipmentID:" << dsi.lmequipmentid().c_str() << ", IP:" << dsi.lmip().c_str();
	}

	theSignalBase.tuning().Sources().sortByID();

	emit sourcesLoaded();

	requestTuningSourcesState();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_GET_TUNING_SOURCES_STATES

void TuningSocket::requestTuningSourcesState()
{
	assert(isClearToSendRequest());

	QThread::msleep(TUNING_SOCKET_TIMEOUT_STATE);

	int serverType = selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return;
	}

	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_TUNING).equipmentID(serverType);
	if (equipmentID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyTuningSourcesState(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_tuningDataSourcesStatesReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
	if (result == false)
	{
		qDebug() << "TuningSocket::replyTuningSourcesState - error: ParseFromArray";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_tuningDataSourcesStatesReply.error() != 0)
	{
		qDebug() << "TuningSocket::replyTuningSourcesState - error: " << m_tuningDataSourcesStatesReply.error();
		assert(m_tuningDataSourcesStatesReply.error() != 0);
		requestTuningSourcesState();
		return;
	}

	int sourceCount = m_tuningDataSourcesStatesReply.tuningsourcesstate_size();

	for (int i = 0; i < sourceCount; i++)
	{
		const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningsourcesstate(i);
		theSignalBase.tuning().Sources().setState(tss.sourceid(), tss);
	}

	requestReadTuningSignals();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_TUNING_SIGNALS_READ

void TuningSocket::requestReadTuningSignals()
{
	assert(isClearToSendRequest());

	int serverType = selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return;
	}

	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_TUNING).equipmentID(serverType);
	if (equipmentID.isEmpty() == true)
	{
		assert(0);
		requestTuningSourcesState();
		return;
	}

	int signalForReadCount = theSignalBase.tuning().Signals().count();
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

		Metrology::Signal* pSignal = theSignalBase.tuning().Signals().signal(i + startIndex);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(param.hash());

		m_readTuningSignalsIndex ++;
	}

	sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyReadTuningSignals(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_readTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
	if (result == false)
	{
		qDebug() << "TuningSocket::replyReadTuningSignals - error: ParseFromArray";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_readTuningSignalsReply.error() != 0)
	{
		qDebug() << "TuningSocket::replyReadTuningSignals - error: " << m_readTuningSignalsReply.error();
		assert(m_readTuningSignalsReply.error() != 0);
		requestTuningSourcesState();
		return;
	}

	int readReplyCount = m_readTuningSignalsReply.tuningsignalstate_size();

	for (int i = 0; i < readReplyCount; i++)
	{
		theSignalBase.tuning().Signals().setState(m_readTuningSignalsReply.tuningsignalstate(i));
	}

	requestWriteTuningSignals();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_TUNING_SIGNALS_WRITE

void TuningSocket::requestWriteTuningSignals()
{
	assert(isClearToSendRequest());

	int serverType = selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return;
	}

	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_TUNING).equipmentID(serverType);
	if (equipmentID.isEmpty() == true)
	{
		assert(0);
		requestTuningSourcesState();
		return;
	}

	int cmdCount = theSignalBase.tuning().cmdFowWriteCount();
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
		TuningWriteCmd cmd = theSignalBase.tuning().cmdFowWrite(i);

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

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyWriteTuningSignals(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestTuningSourcesState();
		return;
	}

	bool result = m_writeTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
	if (result == false)
	{
		qDebug() << "TuningSocket::replyWriteTuningSignals - error: ParseFromArray";
		assert(result);
		requestTuningSourcesState();
		return;
	}

	if (m_writeTuningSignalsReply.error() != 0)
	{
		qDebug() << "TuningSocket::replyWriteTuningSignals - error: " << m_writeTuningSignalsReply.error();
		requestTuningSourcesState();
		return;
	}

	requestTuningSourcesState();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::configurationLoaded()
{
	HostAddressPort addr1 = theOptions.socket().client(SOCKET_TYPE_TUNING).address(SOCKET_SERVER_TYPE_PRIMARY);
	HostAddressPort addr2 = theOptions.socket().client(SOCKET_TYPE_TUNING).address(SOCKET_SERVER_TYPE_RESERVE);

	HostAddressPort currAddr1 = serverAddressPort(SOCKET_SERVER_TYPE_PRIMARY);
	HostAddressPort currAddr2 = serverAddressPort(SOCKET_SERVER_TYPE_RESERVE);

	if (	addr1.address32() == currAddr1.address32() && addr1.port() == currAddr1.port() &&
			addr2.address32() == currAddr2.address32() && addr2.port() == currAddr2.port())
	{
		return;
	}

	setServers(addr1, addr2, true);
}

// -------------------------------------------------------------------------------------------------------------------

