#pragma once

#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>
#include "GrpcSessionGuard.h"

#include <CommonLib/HostAddressPort.h>

#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"

class GrpcServer : public LogWrapper
{
public:
	explicit GrpcServer(const SoftwareInfo& serverSwInfo,
						bool allowAllClients,
						const std::vector<ClientInfo>& clients,
						bool checkHostName,
						const HostAddressPort& listenIP,
						CircularLoggerShared log);

	GrpcServer(const GrpcServer&) = delete;
	GrpcServer& operator=(const GrpcServer&) = delete;
	GrpcServer(GrpcServer&&) = delete;
	GrpcServer& operator=(GrpcServer&&) = delete;

	virtual ~GrpcServer();

	void start();
	void stop();

	HostAddressPort listenIP() const;

	void setSessionTimeout(int seconds);
	bool isBinded() const;
	bool isRunning() const;
	bool isStopRequested() const;

	virtual void eraseAuthToken(const std::string& authToken) { Q_UNUSED(authToken) }

	grpc::Status handshake(grpc::ServerContext* context,
						   const Grpc::HandshakeRequest* request,
						   Grpc::HandshakeReply* reply);

	grpc::Status ping(grpc::ServerContext* context,
					const Grpc::PingRequest* request,
					Grpc::PingReply* reply);

protected:
	virtual grpc::Service* getGrpcService() = 0;
	virtual QString serviceName() const = 0;

protected:
	HostAddressPort m_listenIP;

	std::mutex m_serverMutex;
	std::unique_ptr<grpc::Server> m_server;
	std::atomic_bool m_stopRequested {false};
	std::thread m_thread;
	std::atomic_bool m_threadStarted {false};
	std::atomic_bool m_binded {false};

	GrpcSessionGuard m_sessionGuard;
};

//

template <typename REPLY_TYPE>
bool WriteReplyFunc(grpc::ServerContext* context, grpc::ServerWriter<REPLY_TYPE>* writer,
					const REPLY_TYPE& reply, grpc::Status& wrStatus, CircularLoggerShared log)
{
	wrStatus = grpc::Status::OK;

	if (context->IsCancelled())
	{
		wrStatus = grpc::Status::CANCELLED;

		if (log != nullptr)
		{
			DEBUG_LOG_MSG(log, QString("writeReplyFunc<%1>: context CANCELLED").arg(QString::fromStdString(reply.GetTypeName())));
		}

		return false;
	}

	if (writer->Write(reply) == false)
	{
		if (log != nullptr)
		{
			DEBUG_LOG_MSG(log, QString("writeReplyFunc<%1>: writer->Write returns FALSE").arg(QString::fromStdString(reply.GetTypeName())));
		}

		return false;
	}

	return true;
}

template <typename REPLY_TYPE>
using WriteReplyPtr = bool (*)(grpc::ServerContext*,
							   grpc::ServerWriter<REPLY_TYPE>*,
							   const REPLY_TYPE&,
							   grpc::Status&,
							   CircularLoggerShared);
