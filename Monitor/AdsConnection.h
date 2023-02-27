#pragma once
#include "../OnlineLib/Tcp.h"
#include "../UtilsLib/ILogFile.h"
#include "../UtilsLib/SimpleThread.h"
#include "MonitorConfigController.h"
#include "TcpSignalClient.h"
#include "TcpSignalRecents.h"

// Get signals params and states from AppDataServices(s)
// and set them to
//
class AdsConnection : public QObject
{
	Q_OBJECT

private:
	struct Connection
	{
		Connection(MonitorConfigController& configController,
				   const MonitorSettings::AppDataService& ads,
				   MonitorSignalManager& signalManager,
				   ILogFile* logFile);
		Connection(const Connection&) = delete;
		Connection(Connection&& src) noexcept;
		~Connection();

		Connection& operator=(const Connection&) = delete;
		Connection& operator=(Connection&& src) noexcept;

		void stopAndDestroy();
		HostAddressPort address() const;

		// --
		//
		TcpSignalClient* tcpSignalClient = nullptr;
		SimpleThread* tcpClientThread = nullptr;

		TcpSignalRecents* tcpSignalRecents = nullptr;
		SimpleThread* tcpClientRecentThread = nullptr;
	};

public:
	AdsConnection(MonitorConfigController& configController,
				  MonitorSignalManager& signalManager,
				  ILogFile* logFile);

	std::vector<Tcp::ConnectionState> tcpSignalConnStates() const;
	std::vector<Tcp::ConnectionState> recentSignalConnStates() const;

private slots:
	void configurationArrived(const ConfigSettings& conf);

	// --
	//
private:
	HasLogFile m_logFile;

	MonitorConfigController& m_configController;
	MonitorSignalManager& m_signalManager;

	std::list<Connection> m_conns;
};


