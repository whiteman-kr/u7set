#pragma once

#include "ClientConnStatsStd.h"
#include "ILoggerStd.h"

#include <AppSignalLibStd/IAppSignalUpdater.h>

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include <chrono>
#include <condition_variable>
#include <expected>
#include <thread>

namespace ClientLib
{
	// gRPC channel cache to reuse channels per address and avoid repeated allocations
	//
	class GrpcChannelCache
	{
	public:
		static std::shared_ptr<grpc::Channel> get(const std::string& address, bool forceToCreateNew = false);
	};


	template<typename GrpcServerType>
	class ClientGrpc : public ClientConnStatsStd
	{
	public:
		ClientGrpc(const ::Network::SoftwareInfo& softwareInfo, ServiceEndpoint service, ILoggerStd& log, std::string logPrefix);

		virtual ~ClientGrpc();

		void shutUp();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual void statsReconnect() override;
		virtual std::string statsObjectName() override;
		virtual std::string statsServerId() override;
		virtual ServiceConnectionState statsConnectionState() override;

	private:
		void workerThreadFunc(std::stop_token stoken);

		std::expected<bool, std::string> handshake();

	protected:
		IAppSignalUpdater::SourceIdType sourceId() const
		{
			static_assert(sizeof(IAppSignalUpdater::SourceIdType) >= sizeof(void*), "SourceIdType must be large enough to hold a pointer");
			return reinterpret_cast<IAppSignalUpdater::SourceIdType>(this);
		}

		std::string statusToString(const ::grpc::Status& status);

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

			context.AddMetadata(SESSION_AUTH_TOKEN, token);

			return;
		}

		virtual void clientCommunicationLoop(std::stop_token stoken) = 0;

		ServiceConnectionState tcpState() const
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
		const ::Network::SoftwareInfo m_softwareInfo{}; // Client software info.
		const ServiceEndpoint m_service{};
		ILoggerStd& m_log;
		const std::string m_logPrefix{};

		// --
		//
		std::jthread m_workerThread; // Must be before m_channel and m_stub, so they are destroyed after thread exit.

		std::shared_ptr<grpc::Channel> m_channel;
		std::unique_ptr<typename GrpcServerType::Stub> m_stub;

		mutable std::mutex m_authTokenMutex;
		std::string m_authToken;

		mutable std::mutex m_tcpStateMutex; // Only for m_tcpState
		ServiceConnectionState m_tcpState{};

		static inline const std::string SESSION_AUTH_TOKEN{"session-auth-token"};
	};


	template<typename GrpcServerType>
	ClientGrpc<GrpcServerType>::ClientGrpc(const ::Network::SoftwareInfo& softwareInfo,
										   ServiceEndpoint service,
										   ILoggerStd& log,
										   std::string logPrefix) :
		m_softwareInfo{softwareInfo},
		m_service{service},
		m_log{log},
		m_logPrefix{logPrefix + ">> "}
	{
		m_tcpState.serverEquipmentID = m_service.equipmentId;
		m_tcpState.peerAddr = m_service.address;
		m_tcpState.localSoftwareInfo = m_softwareInfo;

		m_channel = GrpcChannelCache::get(m_service.address, false);
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

		// Give gRPC time to clean up
		// (This is a workaround, not a real fix)
		//
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		return;
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
	std::string ClientGrpc<GrpcServerType>::statsObjectName()
	{
		return "GrpcClient";
	}

	template<typename GrpcServerType>
	std::string ClientGrpc<GrpcServerType>::statsServerId()
	{
		return m_service.equipmentId;
	}

	template<typename GrpcServerType>
	ServiceConnectionState ClientGrpc<GrpcServerType>::statsConnectionState()
	{
		return tcpState();
	}

	template<typename GrpcServerType>
	void ClientGrpc<GrpcServerType>::workerThreadFunc(std::stop_token stoken)
	{
		int transientFailureCount = 0;
		constexpr int MAX_TRANSIENT_FAILURES = 6;

		while (stoken.stop_requested() == false)
		{
			setAuthToken({});

			{
				std::scoped_lock lock{m_tcpStateMutex};
				m_tcpState.isSocketConnected = false;
				m_tcpState.isConnected = false;
				m_tcpState.setConnectionResult = SetConnectionResult2::Undefined;
				m_tcpState.sentBytes = 0;
				m_tcpState.receivedBytes = 0;
				m_tcpState.requestCount = 0;
				m_tcpState.replyCount = 0;
			}

			// Wait until channel becomes READY
			//
			if (grpc_connectivity_state state = m_channel->GetState(true); state != GRPC_CHANNEL_READY)
			{
				std::string_view stateStr;
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

				m_log.writeMessage(m_logPrefix + std::format("Waiting for gRPC channel to become READY, current state {}, service {}",
															 stateStr,
															 m_service.to_string()));

				transientFailureCount = (state == GRPC_CHANNEL_TRANSIENT_FAILURE) ? transientFailureCount + 1 : 0;

				if (state == GRPC_CHANNEL_TRANSIENT_FAILURE && transientFailureCount >= MAX_TRANSIENT_FAILURES)
				{
					m_log.writeMessage(m_logPrefix + std::format("Recreating gRPC channel after {} transient failures, service {}",
																 transientFailureCount,
																 m_service.to_string()));
					// Release old resources first
					//
					m_stub.reset();
					m_channel.reset();

					// Small delay to allow gRPC to clean up
					//
					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					// Create new channel and stub
					//
					m_channel = GrpcChannelCache::get(m_service.address.to_string(), true); // Create a new channel.
					m_stub = GrpcServerType::NewStub(m_channel);
					transientFailureCount = 0;
				}

				// Wait for READY state with timeout.
				//
				for (int i = 0; i < 10; i++)
				{
					bool connected = m_channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(1));

					if (connected == true || stoken.stop_requested() == true)
					{
						break;
					}
				}

				continue;
			}
			else
			{
				std::scoped_lock lock{m_tcpStateMutex};
				m_tcpState.isSocketConnected = true;
			}

			m_log.writeMessage(m_logPrefix + std::format("Connected to {}. Waiting for handshake...", m_service.to_string()));

			// Perform gRPC calls and process data.
			//
			auto handshakeResult = handshake();

			// Add handshake here
			//
			if (handshakeResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix +
								 std::format("Handshake failed with service {} error {}", m_service.to_string(), handshakeResult.error()));


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
				m_tcpState.startTime =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				m_tcpState.setConnectionResult = SetConnectionResult2::Ok;
			}

			m_log.writeMessage(m_logPrefix + std::format("Handshake successful with service {}.", m_service.to_string()));

			try
			{
				clientCommunicationLoop(stoken);
			}
			catch (std::exception& e)
			{
				m_log.writeError(m_logPrefix +
								 std::format("Exception in clientCommunicationLoop for service {}: {}", m_service.to_string(), e.what()));

				// Go to reconnect
				//
				continue;
			}
		} // while (stoken.stop_requested() == false)

		m_log.writeMessage(m_logPrefix + std::format("Exiting worker thread for gRPC client to service {}.", m_service.to_string()));
		return;
	}

	template<typename GrpcServerType>
	std::expected<bool, std::string> ClientGrpc<GrpcServerType>::handshake()
	{
		setAuthToken({});

		grpc::ClientContext context;
		context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));

		Grpc::HandshakeRequest request;
		Grpc::HandshakeReply reply;

		*request.mutable_clientsoftwareinfo() = m_softwareInfo;

		::grpc::Status result = m_stub->Handshake(&context, request, &reply);
		if (result.ok() == false)
		{
			incRequestCount();
			return std::unexpected<std::string>{statusToString(result)};
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
	std::string ClientGrpc<GrpcServerType>::statusToString(const ::grpc::Status& status)
	{
		if (status.ok() == true)
		{
			return "OK";
		}

		std::string_view errorCodeStr{};
		switch (status.error_code())
		{
		case ::grpc::StatusCode::OK:
			errorCodeStr = "OK";
			break;
		case ::grpc::StatusCode::CANCELLED:
			errorCodeStr = "CANCELLED";
			break;
		case ::grpc::StatusCode::UNKNOWN:
			errorCodeStr = "UNKNOWN";
			break;
		case ::grpc::StatusCode::INVALID_ARGUMENT:
			errorCodeStr = "INVALID_ARGUMENT";
			break;
		case ::grpc::StatusCode::DEADLINE_EXCEEDED:
			errorCodeStr = "DEADLINE_EXCEEDED";
			break;
		case ::grpc::StatusCode::NOT_FOUND:
			errorCodeStr = "NOT_FOUND";
			break;
		case ::grpc::StatusCode::ALREADY_EXISTS:
			errorCodeStr = "ALREADY_EXISTS";
			break;
		case ::grpc::StatusCode::PERMISSION_DENIED:
			errorCodeStr = "PERMISSION_DENIED";
			break;
		case ::grpc::StatusCode::UNAUTHENTICATED:
			errorCodeStr = "UNAUTHENTICATED";
			break;
		case ::grpc::StatusCode::RESOURCE_EXHAUSTED:
			errorCodeStr = "RESOURCE_EXHAUSTED";
			break;
		case ::grpc::StatusCode::FAILED_PRECONDITION:
			errorCodeStr = "FAILED_PRECONDITION";
			break;
		case ::grpc::StatusCode::ABORTED:
			errorCodeStr = "ABORTED";
			break;
		case ::grpc::StatusCode::OUT_OF_RANGE:
			errorCodeStr = "OUT_OF_RANGE";
			break;
		case ::grpc::StatusCode::UNIMPLEMENTED:
			errorCodeStr = "UNIMPLEMENTED";
			break;
		case ::grpc::StatusCode::INTERNAL:
			errorCodeStr = "INTERNAL";
			break;
		case ::grpc::StatusCode::UNAVAILABLE:
			errorCodeStr = "UNAVAILABLE";
			break;
		case ::grpc::StatusCode::DATA_LOSS:
			errorCodeStr = "DATA_LOSS";
			break;
		case ::grpc::StatusCode::DO_NOT_USE:
			errorCodeStr = "DO_NOT_USE";
			break;
		default:
			errorCodeStr = "UNKNOWN_STATUS_CODE";
			break;
		}

		return std::format("code: {}, error_message: {}", errorCodeStr, status.error_message());
	}
} // namespace ClientLib
