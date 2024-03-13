#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "./include/ClientLib/AdsSourceStateConnection.h"
#include "./include/ClientLib/TcpAppSourcesState.h"
#include "../UtilsLib/SimpleThread.h"

namespace ClientLib
{
	AdsSourceStateConnection::Connection::Connection(const SoftwareInfo& softwareInfo,
													 const SoftwareEndpoint::AppDataService& ads,
													 ILogFile* logFile)
	{
		tcpAppSourceStateClient = new ClientLib::TcpAppSourcesState{softwareInfo, ads, logFile};
		tcpAppSourceStateThread = new SimpleThread{tcpAppSourceStateClient};
		tcpAppSourceStateThread->start();

		return;
	}

	AdsSourceStateConnection::Connection::~Connection()
	{
		stopAndDestroy();
		return;
	}

	void AdsSourceStateConnection::Connection::stopAndDestroy()
	{
		if (tcpAppSourceStateThread != nullptr)
		{
			tcpAppSourceStateThread->quitAndWait(10000);
			delete tcpAppSourceStateThread;
		}

		tcpAppSourceStateClient = nullptr;
		tcpAppSourceStateThread = nullptr;

		return;
	}

	HostAddressPort AdsSourceStateConnection::Connection::address() const
	{
		Q_ASSERT(tcpAppSourceStateClient);

		return tcpAppSourceStateClient->serverAddressPort1();
	}

	AdsSourceStateConnection::AdsSourceStateConnection(ILogFile* logFile) :
		m_logFile(logFile, "AdsConnection")
	{
		return;
	}

	void AdsSourceStateConnection::updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService)
	{
		m_logFile.writeMessage(QString("updateConnections(), %1 app data services").arg(appDataService.size()));
		createAndStart(softwareInfo, appDataService);
		return;
	}

	std::vector<Tcp::ConnectionState> AdsSourceStateConnection::adsConnectionStates() const
	{
		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			Q_ASSERT(c.tcpAppSourceStateClient);
			states.emplace_back(c.tcpAppSourceStateClient->getConnectionState());
		}

		return states;
	}

	std::vector<ClientLib::AppDataSourceState> AdsSourceStateConnection::appDataSourceStates() const
	{
		std::vector<ClientLib::AppDataSourceState> result;
		result.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			auto states = c.tcpAppSourceStateClient->appDataSourceStates();
			result.insert(result.end(), states.begin(), states.end());
		}

		return result;
	}

	void AdsSourceStateConnection::createAndStart(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService)
	{
		m_conns.clear();	// it will stop all connection threads and destroy them

		for (const SoftwareEndpoint::AppDataService& ads : appDataService)
		{
			auto it = std::find_if(m_conns.begin(), m_conns.end(), [&ads](const Connection& c)
			{
				return c.address() == ads.address;
			});

			if (it != m_conns.end())
			{
				// Such connection already exists
				//
				continue;
			}

			m_conns.emplace_back(softwareInfo, ads, m_logFile.logFile());
		}
	}

}
