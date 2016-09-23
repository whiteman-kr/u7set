#include "TcpTuningClient.h"

namespace Tuning
{

	TcpTuningClient::TcpTuningClient(const HostAddressPort& hostAddr) :
		Tcp::Client(hostAddr)
	{
	}


	void TcpTuningClient::onConnection()
	{
		sendRequest(TDS_GET_TUNING_SOURCES_INFO);
	}


	void TcpTuningClient::onDisconnection()
	{

	}


	void TcpTuningClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case TDS_GET_TUNING_SOURCES_INFO:
			onGetTuningSourcesInfoReply(replyData, replyDataSize);
			break;

		default:
			assert(false);
		}
	}



	void TcpTuningClient::onGetTuningSourcesInfoReply(const char* replyData, quint32 replyDataSize)
	{
		m_getDataSourcesInfoReply.Clear();

		m_getDataSourcesInfoReply.ParseFromArray(replyData, replyDataSize);

		int count = m_getDataSourcesInfoReply.datasourceinfo_size();

		for(int i = 0; i < count; i++)
		{
			Tuning::TuningDataSource source;

			source.setInfo(m_getDataSourcesInfoReply.datasourceinfo(i));
		}
	}

}
