#include "GrpcServer.h"

#include <chrono>
#include <QStringList>

#include <CommonLib/Times.h>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"

GrpcServer::GrpcServer(const SoftwareInfo& serverSwInfo,
						bool allowAllClients,
						const std::vector<ClientInfo>& clients,
						bool checkHostName,
						const HostAddressPort& listenIP,
						CircularLoggerShared log) :
	LogWrapper(log),
	m_sessionGuard(serverSwInfo, allowAllClients, clients, checkHostName),
	m_listenIP(listenIP)
{
}

GrpcServer::~GrpcServer()
{
	if (m_running.load(std::memory_order::relaxed) == true)
	{
		stop();
	}
}

void GrpcServer::start()
{
	if (m_running.load(std::memory_order::relaxed) == true)
	{
		Q_ASSERT(false);
		return;
	}

	m_stopRequested.store(false, std::memory_order_relaxed);
	m_binded.store(false, std::memory_order_relaxed);

	m_thread = std::thread
	{
		[this]()
		{
			try
			{
				int selectedPort = 0;

				while(m_server == nullptr && selectedPort == 0)
				{
					if (m_stopRequested.load(std::memory_order_relaxed))
					{
						logMsg(QString("%1 stopRequested!").arg(serviceName()));
						return;
					}

					selectedPort = 0;

#ifdef VLD_IS_INCLUDED
					::VLDDisable();
#endif
					grpc::ServerBuilder builder;

					builder.AddChannelArgument(GRPC_ARG_ALLOW_REUSEPORT, 0);

					builder.RegisterService(getGrpcService());

					QString ipStr = m_listenIP.addressPortStr();
					builder.AddListeningPort(ipStr.toStdString(), grpc::InsecureServerCredentials(), &selectedPort);

					m_server = builder.BuildAndStart();

#ifdef VLD_IS_INCLUDED
					::VLDEnable();
#endif
					if (m_server == nullptr)
					{
						logErr(QString("%1 (%2) NOT started!").arg(serviceName()).arg(m_listenIP.addressPortStr()));
						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}

					if (selectedPort == 0)
					{
						qDebug() << C_STR(QString("%1 (%2) started, but selectedPort == 0").arg(serviceName()).arg(m_listenIP.addressPortStr()));

						m_server->Shutdown();
						m_server.reset();
						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}

					logMsg(QString("%1 started. Listening address: %2").arg(serviceName()).arg(m_listenIP.addressPortStr()));
					break;
				}

				m_binded.store(true, std::memory_order_relaxed);

#ifdef VLD_IS_INCLUDED
				::VLDDisable();
#endif

				logMsg(QString("%1 in Wait state").arg(serviceName()));
				m_server->Wait();		// unblocked by m_server->Shutdown in destructor

#ifdef VLD_IS_INCLUDED
				::VLDEnable();
#endif
				m_binded.store(false, std::memory_order_relaxed);
				logMsg(QString("%1 finished.").arg(serviceName()));
			}
			catch (std::exception& e)
			{
				logErr(QString("%1 thread exception: %2").arg(serviceName()).arg(QString{e.what()}));
			}
			catch (...)
			{
				logErr(QString("%1 thread unknown exception").arg(serviceName()));
			}
		}
	};

	m_sessionGuard.start();
	m_running.store(true, std::memory_order::relaxed);
}

void GrpcServer::stop()
{
	if (m_running.load(std::memory_order::relaxed) == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_sessionGuard.stop();

	m_stopRequested.store(true, std::memory_order_relaxed);

	if (m_server)
	{
		m_server->Shutdown();
	}

	if (m_thread.joinable())
	{
		m_thread.join();
	}

	if (m_server)
	{
		m_server.reset();
	}

	m_running.store(false, std::memory_order::relaxed);
}

HostAddressPort GrpcServer::listenIP() const
{
	return m_listenIP;
}

void GrpcServer::setSessionTimeout(int seconds)
{
	m_sessionGuard.setSessionTimeout(seconds);
}

bool GrpcServer::isBinded() const
{
	return m_binded.load(std::memory_order_relaxed);
}

bool GrpcServer::isRunning() const
{
	return m_running.load(std::memory_order_relaxed);
}

grpc::Status GrpcServer::handshake(grpc::ServerContext* context,
									const Grpc::HandshakeRequest* request,
									Grpc::HandshakeReply* reply)
{
	Q_UNUSED(context);

	grpc::Status status = m_sessionGuard.handshake(request, reply);

	if (status.ok() == true)
	{
		reply->set_serverip(m_listenIP.address32());
		reply->set_serverport(m_listenIP.port());
	}

	return status;
}

grpc::Status GrpcServer::ping(grpc::ServerContext* context,
							const Grpc::PingRequest* request,
							Grpc::PingReply* reply)
{
	Q_UNUSED(context);

	grpc::Status status = m_sessionGuard.ping(request, reply);

	return status;
}


