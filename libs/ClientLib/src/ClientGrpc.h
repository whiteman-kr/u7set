#pragma once

#include "../OnlineLib/TcpClientStatistics.h"
#include <ClientLib/IAppSignalUpdater.h>
#include <CommonLib/ConstStrings.h>

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include <expected>
#include <thread>

namespace ClientLib
{
	// gRPC channel cache to reuse channels per address and avoid repeated allocations
	//
	class GrpcChannelCache
	{
	public:
		static std::shared_ptr<grpc::Channel> get(const std::string& address);
	};


	template<typename GrpcServerType>
	class ClientGrpc : public ClientConnectionStatistics
	{
	public:
		ClientGrpc(const SoftwareInfo& softwareInfo,
				   QString serviceEquipmentId,
				   HostAddressPort serviceAddress,
				   ILogFile& log,
				   QString logPrefix);

		virtual ~ClientGrpc();

		void shutUp();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual void statsReconnect() override;
		virtual QString statsObjectName() override;
		virtual QString statsServerId() override;
		virtual Tcp::ConnectionState statsConnectionState() override;

	private:
		void workerThreadFunc(std::stop_token stoken);

		std::expected<bool, QString> handshake();

	protected:
		IAppSignalUpdater::SourceIdType sourceId() const
		{
			static_assert(sizeof(IAppSignalUpdater::SourceIdType) >= sizeof(void*), "SourceIdType must be large enough to hold a pointer");
			return reinterpret_cast<IAppSignalUpdater::SourceIdType>(this);

			// auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
			// return static_cast<IAppSignalUpdater::SourceIdType>(h);
		}

		QString statusToString(const ::grpc::Status& status);

		void createAuthContext(grpc::ClientContext& context, std::chrono::milliseconds timeout)
		{
			createAuthContext(context);
			context.set_deadline(std::chrono::system_clock::now() + timeout);
			return;
		}

		void createAuthContext(grpc::ClientContext& context)
		{
			auto token = authToken();
			assert(token.empty() == false);

			context.AddMetadata(::Grpc::SESSION_AUTH_TOKEN, token);

			return;
		}

		virtual void clientCommunicationLoop(std::stop_token stoken) = 0;

		Tcp::ConnectionState tcpState() const
		{
			std::lock_guard lock{m_tcpStateMutex};
			return m_tcpState;
		}

	protected:
		std::string authToken() const
		{
			std::scoped_lock locker{m_authTokenMutex};
			return m_authToken;
		}

		void setAuthToken(const std::string& authToken)
		{
			std::scoped_lock locker{m_authTokenMutex};
			m_authToken = authToken;
		}

		void incRequestReplyCount()
		{
			std::scoped_lock lock{m_tcpStateMutex};
			m_tcpState.requestCount++;
			m_tcpState.replyCount++;
		}

		void incRequestCount()
		{
			std::scoped_lock lock{m_tcpStateMutex};
			m_tcpState.requestCount++;
		}

		void incReplyCount()
		{
			std::scoped_lock lock{m_tcpStateMutex};
			m_tcpState.replyCount++;
		}

	protected:
		SoftwareInfo m_softwareInfo; // Client software info.
		QString m_serviceEquipmentId;
		HostAddressPort m_serviceAddress;
		ILogFile& m_log;
		QString m_logPrefix;

		// --
		//
		std::jthread m_workerThread; // Must be before m_channel and m_stub, so they are destroyed after thread exit.

		std::shared_ptr<grpc::Channel> m_channel;
		std::unique_ptr<typename GrpcServerType::Stub> m_stub;

		mutable std::mutex m_authTokenMutex;
		std::string m_authToken;

		mutable std::mutex m_tcpStateMutex; // Only for m_tcpState
		Tcp::ConnectionState m_tcpState{};
	};


	template<typename GrpcServerType>
	ClientGrpc<GrpcServerType>::ClientGrpc(const SoftwareInfo& softwareInfo,
										   QString serviceEquipmentId,
										   HostAddressPort serviceAddress,
										   ILogFile& log,
										   QString logPrefix) :
		m_softwareInfo{softwareInfo},
		m_serviceEquipmentId{serviceEquipmentId},
		m_serviceAddress{serviceAddress},
		m_log{log},
		m_logPrefix{logPrefix + ">> "}
	{
		m_tcpState.serverEquipmentID = m_serviceEquipmentId;
		m_tcpState.peerAddr = m_serviceAddress;
		m_tcpState.localSoftwareInfo = m_softwareInfo;

#if 0
			m_channel = GrpcChannelCache::get(m_serviceAddress);
#else
		int Using_fixed_port_for_AppDataService_gRPC_client;
		auto addressPort = QString{"%1:%2"}.arg(m_serviceAddress.addressStr()).arg(PORT_APP_DATA_SERVICE_GRPC_CLIENT_REQUEST);
		m_channel = GrpcChannelCache::get(addressPort.toStdString());
#endif
		m_stub = GrpcServerType::NewStub(m_channel);

		m_workerThread = std::jthread(
			[this](std::stop_token stoken)
			{
				workerThreadFunc(stoken);
			});
	}

	template<typename GrpcServerType>
	ClientGrpc<GrpcServerType>::~ClientGrpc()
	{
		shutUp();
	}

	template<typename GrpcServerType>
	void ClientGrpc<GrpcServerType>::shutUp()
	{
		if (m_workerThread.joinable() == true)
		{
			m_workerThread.request_stop();
			m_workerThread.join();
		}

		m_stub.reset();
		m_channel.reset();

		return;
	}

	template<typename GrpcServerType>
	void ClientGrpc<GrpcServerType>::statsReconnect()
	{
		m_log.writeWarning(m_logPrefix + "Reconnecting to gRPC service is not supported.");
		return;
	}

	template<typename GrpcServerType>
	QString ClientGrpc<GrpcServerType>::statsObjectName()
	{
		return "GrpcClient";
	}

	template<typename GrpcServerType>
	QString ClientGrpc<GrpcServerType>::statsServerId()
	{
		return m_serviceEquipmentId;
	}

	template<typename GrpcServerType>
	Tcp::ConnectionState ClientGrpc<GrpcServerType>::statsConnectionState()
	{
		return tcpState();
	}

	template<typename GrpcServerType>
	void ClientGrpc<GrpcServerType>::workerThreadFunc(std::stop_token stoken)
	{
		while (stoken.stop_requested() == false)
		{
			setAuthToken({});

			{
				std::scoped_lock lock{m_tcpStateMutex};
				m_tcpState.isSocketConnected = false;
				m_tcpState.isConnected = false;
				m_tcpState.sentBytes = 0;
				m_tcpState.receivedBytes = 0;
				m_tcpState.requestCount = 0;
				m_tcpState.replyCount = 0;
			}

			// Wait until channel becomes READY
			//
			if (grpc_connectivity_state state = m_channel->GetState(true); state != GRPC_CHANNEL_READY)
			{
				QString stateStr;
				switch (state)
				{
				case GRPC_CHANNEL_IDLE:
					stateStr = "GRPC_CHANNEL_IDLE";
					break;
				case GRPC_CHANNEL_CONNECTING:
					stateStr = "GRPC_CHANNEL_CONNECTING";
					break;
				case GRPC_CHANNEL_READY:
					stateStr = "GRPC_CHANNEL_READY";
					break;
				case GRPC_CHANNEL_TRANSIENT_FAILURE:
					stateStr = "GRPC_CHANNEL_TRANSIENT_FAILURE";
					break;
				case GRPC_CHANNEL_SHUTDOWN:
					stateStr = "GRPC_CHANNEL_SHUTDOWN";
					break;
				}

				m_log.writeText(m_logPrefix + QString{"Waiting for gRPC channel to become READY, current state %1, service %2, address %3"}
												  .arg(stateStr)
												  .arg(m_serviceEquipmentId)
												  .arg(m_serviceAddress));

				m_channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(2));
				continue;
			}
			else
			{
				std::scoped_lock lock{m_tcpStateMutex};
				m_tcpState.isSocketConnected = true;
			}

			m_log.writeMessage(
				m_logPrefix +
				QString{"Connected to %1 (gRPC server: %2). Waiting for handshake..."}.arg(m_serviceEquipmentId).arg(m_serviceAddress));

			// Perform gRPC calls and process data.
			//
			auto handshakeResult = handshake();

			// Add handshake here
			//
			if (handshakeResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix + QString{"Handshake failed with service %1, address %2, error %3"}
												   .arg(m_serviceEquipmentId)
												   .arg(m_serviceAddress)
												   .arg(handshakeResult.error()));

				// Just sleep for a while and try to reconnect.
				//
				std::mutex m;
				std::condition_variable_any cv;
				std::unique_lock locker(m);
				cv.wait_for(locker,
							stoken,
							std::chrono::seconds(5),
							[&stoken]()
							{
								return stoken.stop_requested();
							});
				continue;
			}
			else
			{
				std::scoped_lock lock{m_tcpStateMutex};
				m_tcpState.isConnected = true;
				m_tcpState.startTime = QDateTime::currentMSecsSinceEpoch();
			}

			m_log.writeMessage(
				m_logPrefix +
				QString{"Handshake successful with service %1 at address %2."}.arg(m_serviceEquipmentId).arg(m_serviceAddress));

			try
			{
				clientCommunicationLoop(stoken);
			}
			catch (std::exception& e)
			{
				m_log.writeError(m_logPrefix + QString{"Exception in clientCommunicationLoop for service %1 at address %2: %3"}
												   .arg(m_serviceEquipmentId)
												   .arg(m_serviceAddress)
												   .arg(e.what()));
				// Go to reconnect
				//
				continue;
			}
		} // while (stoken.stop_requested() == false)

		m_log.writeMessage(
			m_logPrefix +
			QString{"Exiting worker thread for gRPC client to service %1 at address %2."}.arg(m_serviceEquipmentId).arg(m_serviceAddress));
		return;
	}

	template<typename GrpcServerType>
	std::expected<bool, QString> ClientGrpc<GrpcServerType>::handshake()
	{
		setAuthToken({});

		grpc::ClientContext context;
		context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));

		Grpc::HandshakeRequest request;
		Grpc::HandshakeReply reply;

		m_softwareInfo.serializeTo(request.mutable_clientsoftwareinfo());

		::grpc::Status result = m_stub->Handshake(&context, request, &reply);
		if (result.ok() == false)
		{
			incRequestCount();
			return std::unexpected<QString>{statusToString(result)};
		}
		else
		{
			incRequestReplyCount();
		}

		assert(reply.authtoken().empty() == false);
		setAuthToken(reply.authtoken());

		return true;
	}

	template<typename GrpcServerType>
	QString ClientGrpc<GrpcServerType>::statusToString(const ::grpc::Status& status)
	{
		if (status.ok() == true)
		{
			return QStringLiteral("OK");
		}

		QString errorCodeStr;
		switch (status.error_code())
		{
		case ::grpc::StatusCode::OK:
			errorCodeStr = QStringLiteral("OK");
			break;
		case ::grpc::StatusCode::CANCELLED:
			errorCodeStr = QStringLiteral("CANCELLED");
			break;
		case ::grpc::StatusCode::UNKNOWN:
			errorCodeStr = QStringLiteral("UNKNOWN");
			break;
		case ::grpc::StatusCode::INVALID_ARGUMENT:
			errorCodeStr = QStringLiteral("INVALID_ARGUMENT");
			break;
		case ::grpc::StatusCode::DEADLINE_EXCEEDED:
			errorCodeStr = QStringLiteral("DEADLINE_EXCEEDED");
			break;
		case ::grpc::StatusCode::NOT_FOUND:
			errorCodeStr = QStringLiteral("NOT_FOUND");
			break;
		case ::grpc::StatusCode::ALREADY_EXISTS:
			errorCodeStr = QStringLiteral("ALREADY_EXISTS");
			break;
		case ::grpc::StatusCode::PERMISSION_DENIED:
			errorCodeStr = QStringLiteral("PERMISSION_DENIED");
			break;
		case ::grpc::StatusCode::UNAUTHENTICATED:
			errorCodeStr = QStringLiteral("UNAUTHENTICATED");
			break;
		case ::grpc::StatusCode::RESOURCE_EXHAUSTED:
			errorCodeStr = QStringLiteral("RESOURCE_EXHAUSTED");
			break;
		case ::grpc::StatusCode::FAILED_PRECONDITION:
			errorCodeStr = QStringLiteral("FAILED_PRECONDITION");
			break;
		case ::grpc::StatusCode::ABORTED:
			errorCodeStr = QStringLiteral("ABORTED");
			break;
		case ::grpc::StatusCode::OUT_OF_RANGE:
			errorCodeStr = QStringLiteral("OUT_OF_RANGE");
			break;
		case ::grpc::StatusCode::UNIMPLEMENTED:
			errorCodeStr = QStringLiteral("UNIMPLEMENTED");
			break;
		case ::grpc::StatusCode::INTERNAL:
			errorCodeStr = QStringLiteral("INTERNAL");
			break;
		case ::grpc::StatusCode::UNAVAILABLE:
			errorCodeStr = QStringLiteral("UNAVAILABLE");
			break;
		case ::grpc::StatusCode::DATA_LOSS:
			errorCodeStr = QStringLiteral("DATA_LOSS");
			break;
		case ::grpc::StatusCode::DO_NOT_USE:
			errorCodeStr = QStringLiteral("DO_NOT_USE");
			break;
		default:
			errorCodeStr = QStringLiteral("UNKNOWN_STATUS_CODE");
			break;
		}

		return QString{"code: %1, error_message: %2"}.arg(errorCodeStr).arg(QString::fromStdString(status.error_message()));
	}
} // namespace ClientLib
