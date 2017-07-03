#pragma once

#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include "TuningSource.h"


namespace Tuning
{

	class TcpTuningClient : public Tcp::Client
	{
	public:
		TcpTuningClient(const HostAddressPort& hostAddr, const QString equipmentID);

	private:
		virtual void onConnection() override;
		virtual void onDisconnection() override;

		void onGetTuningSourcesInfoReply(const char* replyData, quint32 replyDataSize);

		void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize);


	private:
		Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	};


}
