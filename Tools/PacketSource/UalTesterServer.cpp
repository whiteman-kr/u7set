#include "UalTesterServer.h"

#include "../../lib/TuningValue.h"

// -------------------------------------------------------------------------------
//
// TcpTuningServer class implementation
//
// -------------------------------------------------------------------------------

UalTesterServer::UalTesterServer(const SoftwareInfo& sotwareInfo, SignalBase* signalBase) :
	Tcp::Server(sotwareInfo),
	m_sotwareInfo(sotwareInfo),
	m_signalBase(signalBase)
{
}

void UalTesterServer::onServerThreadStarted()
{
}

void UalTesterServer::onServerThreadFinished()
{
}

void UalTesterServer::onConnection()
{
	emit connectionChanged(true);
}

void UalTesterServer::onDisconnection()
{
	emit connectionChanged(false);
}

void UalTesterServer::clientConnectionChanged(bool isConnect)
{
	emit connectionChanged(isConnect);
}

void UalTesterServer::clientSignalStateChanged(Hash hash, double prevState, double state)
{
	emit signalStateChanged(hash, prevState, state);
}

Tcp::Server* UalTesterServer::getNewInstance()
{
	UalTesterServer* newServer = new UalTesterServer(m_sotwareInfo, m_signalBase);

	connect(newServer, &UalTesterServer::connectionChanged, this, &UalTesterServer::clientConnectionChanged, Qt::QueuedConnection);
	connect(newServer, &UalTesterServer::signalStateChanged, this, &UalTesterServer::clientSignalStateChanged, Qt::QueuedConnection);

	return newServer;
}

void UalTesterServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
		case TDS_GET_TUNING_SOURCES_INFO:
			onGetTuningSourcesInfoRequest(requestData, requestDataSize);
			break;

		case TDS_GET_TUNING_SOURCES_STATES:
			onGetTuningSourcesStateRequest(requestData, requestDataSize);
			break;

		case TDS_TUNING_SIGNALS_WRITE:
			onTuningSignalsWriteRequest(requestData, requestDataSize);
			break;
	}
}

void UalTesterServer::onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize)
{
	m_getTuningSourcesInfoReply.Clear();

	bool result = m_getTuningSourcesInfo.ParseFromArray(requestData, static_cast<int>(requestDataSize));

	if (result == false)
	{
		m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getTuningSourcesInfoReply);
		return;
	}

	m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getTuningSourcesInfoReply);
}

void UalTesterServer::onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize)
{
	m_getTuningSourcesStatesReply.Clear();

	bool result = m_getTuningSourcesStates.ParseFromArray(requestData, static_cast<int>(requestDataSize));

	if (result == false)
	{
		m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getTuningSourcesStatesReply);
		return;
	}

	m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getTuningSourcesStatesReply);
}

void UalTesterServer::onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize)
{
	m_tuningSignalsWriteReply.Clear();

	bool result = m_tuningSignalsWriteRequest.ParseFromArray(requestData, static_cast<int>(requestDataSize));
	if (result == false)
	{
		m_tuningSignalsWriteReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_tuningSignalsWriteReply);
		return;
	}

	int cmdCount = m_tuningSignalsWriteRequest.commands_size();
	for (int i = 0; i < cmdCount; i++)
	{
		Network::TuningWriteCommand wrCmd = m_tuningSignalsWriteRequest.commands(i);
		TuningValue value(wrCmd.value());

		if (m_signalBase == nullptr)
		{
			continue;
		}

		PS::Signal* pSignal = m_signalBase->signalPtr(static_cast<Hash>(wrCmd.signalhash()));
		if (pSignal == nullptr)
		{
			qDebug() << "Error: Signal " << pSignal->appSignalID().toLocal8Bit().constData() << " - is not found";
			continue;
		}

		double prevState = pSignal->state();										// get prev state of signal

		result = pSignal->setState(value.toDouble());								// set new state of signal
		if (result == true)
		{
			qDebug() << "Set state " << pSignal->appSignalID().toLocal8Bit().constData() << "=" << value.toDouble();
			emit signalStateChanged(pSignal->hash(), prevState, pSignal->state());	// write to history log
		}
		else
		{
			qDebug() << "Error: Set state " << pSignal->appSignalID().toLocal8Bit().constData() << "=" << value.toDouble();
		}
	}

	m_tuningSignalsWriteReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_tuningSignalsWriteReply);
}

// -------------------------------------------------------------------------------
//
// TcpTuningServerThread class implementation
//
// -------------------------------------------------------------------------------

UalTesterServerThread::UalTesterServerThread(const HostAddressPort& listenAddressPort, UalTesterServer* server, std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{
}
