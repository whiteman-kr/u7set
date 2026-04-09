#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include <AppSignalLibStd/IAppSignalUpdater.h>
#include <AppSignalLibStd/IRecentAppSignals.h>


//		ADS_GET_APP_SIGNAL_STATE <------+
//				|						|			Repeat it
//				+------------------------
//

namespace ClientLib
{

	class TcpSignalRecents : public Tcp::Client,
							 public TcpClientStatistics,
							 public HasLogFile
	{
		Q_OBJECT

	public:
		TcpSignalRecents(const SoftwareInfo& softwareInfo,
						 const SoftwareEndpoint::AppDataService& adsInfo,
						 IRecentAppSignals& recentAppSignals,
						 IAppSignalUpdater& signalUpdater,
						 ILogFile* logFile);
		virtual ~TcpSignalRecents();

	public:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;
		virtual void onConnection() override;
		virtual void onDisconnection() override;
		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	protected:
		void requestSignalState();
		void processSignalState(const QByteArray& data);

	private:
		SoftwareEndpoint::AppDataService m_serverSettings;
		IRecentAppSignals& m_recentAppSignals;
		IAppSignalUpdater& m_signalUpdater;
	};

} // namespace ClientLib
