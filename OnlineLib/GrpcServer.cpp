#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

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
	m_sessionGuard(serverSwInfo, allowAllClients, clients, checkHostName, *this),
	m_listenIP(listenIP)
{
}

GrpcServer::~GrpcServer()
{
	Q_ASSERT(m_threadStarted.load(std::memory_order_acquire) == false);		// server must be stopped
}

void GrpcServer::start()
{
	if (m_threadStarted.load(std::memory_order::relaxed) == true)
	{
		Q_ASSERT(false);
		return;
	}

	m_stopRequested.store(false, std::memory_order_relaxed);
	m_binded.store(false, std::memory_order_relaxed);
	m_threadStarted.store(true, std::memory_order::relaxed);

	m_sessionGuard.start();

	m_thread = std::thread
	{
		[this]()
		{
			try
			{
				int selectedPort = 0;
				grpc::Server* serverRaw = nullptr;

				while(selectedPort == 0)
				{
					if (isStopRequested())
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

					{
						std::lock_guard lg(m_serverMutex);
						m_server = builder.BuildAndStart();
						serverRaw = m_server.get();
					}

#ifdef VLD_IS_INCLUDED
					::VLDEnable();
#endif
					if (serverRaw == nullptr)
					{
						logErr(QString("%1 (%2) NOT started!").arg(serviceName()).arg(m_listenIP.addressPortStr()));
						std::this_thread::sleep_for(std::chrono::seconds(2));
						continue;
					}

					if (selectedPort == 0)
					{
						qDebug() << C_STR(QString("%1 (%2) started, but selectedPort == 0").arg(serviceName()).arg(m_listenIP.addressPortStr()));

						{
							std::lock_guard lg(m_serverMutex);

							if (m_server != nullptr)
							{
								m_server->Shutdown();
								m_server.reset();
								serverRaw = nullptr;
							}
						}

						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}

					logMsg(QString("%1 started. Listening address: %2").arg(serviceName()).arg(m_listenIP.addressPortStr()));
					break;
				}

#ifdef VLD_IS_INCLUDED
				::VLDDisable();
#endif
				if (serverRaw != nullptr)
				{
					m_binded.store(true, std::memory_order_relaxed);
					logMsg(QString("%1 in Wait state").arg(serviceName()));
					serverRaw->Wait();		// unblocked by m_server->Shutdown in stop
				}

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
}

void GrpcServer::stop()
{
	if (m_threadStarted.exchange(false, std::memory_order::acquire) == false)
	{
		return;
	}

	m_stopRequested.store(true, std::memory_order_relaxed);

	grpc::Server* serverRaw = nullptr;

	{
		std::lock_guard lg(m_serverMutex);
		serverRaw = m_server.get();
	}

	if (serverRaw != nullptr)
	{
		serverRaw->Shutdown();
	}

	if (m_thread.joinable())
	{
		m_thread.join();
	}

	m_sessionGuard.stop();

	{
		std::lock_guard lg(m_serverMutex);
		m_server.reset();
	}
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
	return m_threadStarted.load(std::memory_order_relaxed);
}

bool GrpcServer::isStopRequested() const
{
	return m_stopRequested.load(std::memory_order_relaxed);
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


