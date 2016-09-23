#pragma once

#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include "TuningDataSource.h"

namespace Tuning
{

	class TcpTuningServerThread;

	// -------------------------------------------------------------------------------
	//
	// TcpTuningDataServer class declaration
	//
	// -------------------------------------------------------------------------------

	class TcpTuningServer  : public Tcp::Server
	{
	public:
		TcpTuningServer();

		void setThread(TcpTuningServerThread* thread);

	private:
		virtual void onServerThreadStarted() override;
		virtual void onServerThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		Tcp::Server* getNewInstance() override;

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

		void onGetTuningSourcesInfoRequest();
		void onGetTuningSourcesStateRequest();

		TuningDataSources& tuningSources();

	private:
		TcpTuningServerThread* m_thread = nullptr;

		Network::GetDataSourcesInfoReply m_getTuningSourcesInfoReply;
	};



	// -------------------------------------------------------------------------------
	//
	// TcpAppDataServerThread class declaration
	//
	// -------------------------------------------------------------------------------

	class TcpTuningServerThread : public Tcp::ServerThread
	{
	public:
		TcpTuningServerThread(const HostAddressPort& listenAddressPort,
								TcpTuningServer* server,
								TuningDataSources& tuningSources);

		TuningDataSources& tuningSources();

	private:
		TuningDataSources& m_tuningSources;
	};

}
