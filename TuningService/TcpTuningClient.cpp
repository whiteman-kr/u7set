#include "TcpTuningClient.h"
#include "version.h"

namespace Tuning
{

	TcpTuningClient::TcpTuningClient(const SoftwareInfo& softwareInfo, const HostAddressPort& hostAddr) :
		Tcp::Client(softwareInfo, hostAddr)
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
			Tuning::TuningSource source;

			source.setInfo(m_getDataSourcesInfoReply.datasourceinfo(i));

			QString str = QString("Tuning source '%1', %2").arg(source.lmAdapterID()).arg(source.lmAddressStr());

			qDebug() << C_STR(str);
		}
	}

}
