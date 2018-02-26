#pragma once

#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include "TuningSource.h"


namespace Tuning
{

	class TcpTuningClient : public Tcp::Client
	{
	public:
		TcpTuningClient(const SoftwareInfo& softwareInfo, const HostAddressPort& hostAddr);

	private:
		virtual void onConnection() override;
		virtual void onDisconnection() override;

		void onGetTuningSourcesInfoReply(const char* replyData, quint32 replyDataSize);

		void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize);


	private:
		Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	};


}
