#include "TcpAppDataServer.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer(const SoftwareInfo& softwareInfo,
								   Archive* archive) :
	Tcp::Server(softwareInfo, "ArchiveAppDataServer"),
	m_archive(archive)
{
}

Tcp::Server* TcpAppDataServer::getNewInstance()
{
	return new TcpAppDataServer(localSoftwareInfo(), m_archive);
}

void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
	case ARCHS_SAVE_APP_SIGNALS_STATES:
		onSaveAppSignalsStatesToArchive(requestData, requestDataSize);
		break;

	default:
		logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
	}
}

void TcpAppDataServer::onSaveAppSignalsStatesToArchive(const char* requestData, quint32 requestDataSize)
{
	bool result = m_saveStatesRequest.ParseFromArray(requestData, requestDataSize);

	m_saveStatesReply.Clear();

	if (result == false)
	{
		m_saveStatesReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_saveStatesReply);
		return;
	}

	//
	// Check m_saveStatesRequest.clientequipmentid() here!
	//

	int statesCount = m_saveStatesRequest.appsignalstates_size();

	SimpleAppSignalState state;

	for(int i = 0; i < statesCount; i++)
	{
		state.load(m_saveStatesRequest.appsignalstates(i));

		m_archive->saveState(state);
	}

	m_saveStatesReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(m_saveStatesReply);
}
