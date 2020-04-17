#include "UalTesterServer.h"

#include "../../lib/TuningValue.h"

// -------------------------------------------------------------------------------
//
// TcpTuningServer class implementation
//
// -------------------------------------------------------------------------------

UalTesterServer::UalTesterServer(const SoftwareInfo& sotwareInfo, SourceBase* sourceBase, SignalBase* signalBase) :
	Tcp::Server(sotwareInfo),
	m_sotwareInfo(sotwareInfo),
	m_sourceBase(sourceBase),
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

void UalTesterServer::clientExitApplication()
{
	emit exitApplication();
}

Tcp::Server* UalTesterServer::getNewInstance()
{
	if (m_sourceBase == nullptr || m_signalBase == nullptr)
	{
		return nullptr;
	}

	UalTesterServer* newServer = new UalTesterServer(m_sotwareInfo, m_sourceBase, m_signalBase);

	connect(newServer, &UalTesterServer::connectionChanged, this, &UalTesterServer::clientConnectionChanged, Qt::QueuedConnection);
	connect(newServer, &UalTesterServer::signalStateChanged, this, &UalTesterServer::clientSignalStateChanged, Qt::QueuedConnection);
	connect(newServer, &UalTesterServer::exitApplication, this, &UalTesterServer::clientExitApplication, Qt::QueuedConnection);

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

		case TDS_DATA_SOURCE_WRITE:
			onDataSourceWriteRequest(requestData, requestDataSize);
			break;

		case TDS_PACKETSOURCE_EXIT:
			onPacketSourceExitRequest(requestData, requestDataSize);
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

	if (m_sourceBase != nullptr)
	{
		int sourceCount = m_sourceBase->count();
		for (int i = 0; i < sourceCount; i++)
		{
			PS::Source* pSource = m_sourceBase->sourcePtr(i);
			if (pSource == nullptr)
			{
				continue;
			}

			Network::DataSourceInfo* protoInfo = m_getTuningSourcesInfoReply.add_tuningsourceinfo();

			protoInfo->set_id(static_cast<quint64>(i));
			protoInfo->set_lmequipmentid(pSource->info().equipmentID.toStdString());
			protoInfo->set_lmcaption(pSource->info().caption.toStdString());
		}
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
			continue;
		}

		double prevState = pSignal->state();										// get prev state of signal

		bool res = pSignal->setState(value.toDouble());								// set new state of signal
		if (res == true)
		{
			qDebug() << "Set state " << pSignal->appSignalID() << "=" << value.toDouble();
			emit signalStateChanged(pSignal->hash(), prevState, pSignal->state());	// write to history log
		}
		else
		{
			qDebug() << "Error: Set state " << pSignal->appSignalID() << "=" << value.toDouble();
		}
	}

	m_tuningSignalsWriteReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_tuningSignalsWriteReply);
}

void UalTesterServer::onDataSourceWriteRequest(const char *requestData, quint32 requestDataSize)
{
	m_dataSourceWriteReply.Clear();

	bool result = m_dataSourceWriteRequest.ParseFromArray(requestData, static_cast<int>(requestDataSize));
	if (result == false)
	{
		m_dataSourceWriteReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_dataSourceWriteReply);
		return;
	}

	if (m_sourceBase != nullptr)
	{
		QString sourceID = QString::fromStdString(m_dataSourceWriteRequest.sourceequipmentid());
		bool state = m_dataSourceWriteRequest.state();

		PS::Source* pSource = m_sourceBase->sourcePtr(sourceID);
		if (pSource == nullptr)
		{
			qDebug() << "Error: Source" << sourceID << "in not found";
		}
		else
		{
			if (state == true)
			{
				bool res = pSource->run();
				if (res == false)
				{
					qDebug() << "Error: Run source" << sourceID;
				}
			}
			else
			{
				bool res = pSource->stop();
				if (res == false)
				{
					qDebug() << "Stop source" << sourceID << "- source is already stopped";
				}
			}
		}
	}

	m_dataSourceWriteReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_dataSourceWriteReply);
}

void UalTesterServer::onPacketSourceExitRequest(const char *requestData, quint32 requestDataSize)
{
	m_packetSourceExitReply.Clear();

	bool result = m_packetSourceExitRequest.ParseFromArray(requestData, static_cast<int>(requestDataSize));
	if (result == false)
	{
		m_packetSourceExitReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_packetSourceExitReply);
		return;
	}

	emit exitApplication();

	m_packetSourceExitReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_packetSourceExitReply);
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
