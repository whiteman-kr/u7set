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
	m_sessionGuard(serverSwInfo, allowAllClients, clients, checkHostName),
	m_log(log)
{
	initService(listenIP);
}

GrpcServer::~GrpcServer()
{
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
}

void GrpcServer::setSessionTimeout(int seconds)
{
	m_sessionGuard.setSessionTimeout(seconds);
}

bool GrpcServer::isBinded() const
{
	return m_binded.load(std::memory_order_relaxed);
}

grpc::Status GrpcServer::Handshake(grpc::ServerContext* context,
									   const Grpc::HandshakeRequest* request,
									   Grpc::HandshakeReply* reply)
{
	Q_UNUSED(context);

	return m_sessionGuard.handshake(request, reply);
}

void GrpcServer::initService(const HostAddressPort& listenIP)
{
	m_stopRequested.store(false, std::memory_order_relaxed);
	m_binded.store(false, std::memory_order_relaxed);

	m_thread = std::thread
	{
		[this, listenIP]()
		{
			try
			{
				int selectedPort = 0;

				while(m_server == nullptr && selectedPort == 0)
				{
					if (m_stopRequested.load(std::memory_order_relaxed))
					{
						DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv stopRequested!"));
						return;
					}

					selectedPort = 0;

#ifdef VLD_IS_INCLUDED
					::VLDDisable();
#endif
					grpc::ServerBuilder builder;

					builder.AddChannelArgument(GRPC_ARG_ALLOW_REUSEPORT, 0);

					builder.RegisterService(this);

					QString ipStr = listenIP.addressPortStr();
					builder.AddListeningPort(ipStr.toStdString(), grpc::InsecureServerCredentials(), &selectedPort);

					m_server = builder.BuildAndStart();

#ifdef VLD_IS_INCLUDED
					::VLDEnable();
#endif
					if (m_server == nullptr)
					{
						DEBUG_LOG_ERR(m_log, QString("GrpcAppDataSrv (%1) NOT started!").arg(listenIP.addressPortStr()));
						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}

					if (selectedPort == 0)
					{
						qDebug() << C_STR(QString("GrpcAppDataSrv (%1) started, but selectedPort == 0").arg(listenIP.addressPortStr()));

						m_server->Shutdown();
						m_server.reset();
						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}

					DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv started. Listening address: %1").arg(listenIP.addressPortStr()));
					break;
				}

				m_binded.store(true, std::memory_order_relaxed);

#ifdef VLD_IS_INCLUDED
				::VLDDisable();
#endif

				DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv in Wait state"));
				m_server->Wait();		// unblocked by m_server->Shutdown in destructor

#ifdef VLD_IS_INCLUDED
				::VLDEnable();
#endif
				m_binded.store(false, std::memory_order_relaxed);
				DEBUG_LOG_MSG(m_log, "GrpcAppDataSrv finished.");
			}
			catch (std::exception& e)
			{
				DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread exception: " + QString{e.what()});
			}
			catch (...)
			{
				DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread unknown exception");
			}
		}
	};

	m_sessionGuard.start();
}
