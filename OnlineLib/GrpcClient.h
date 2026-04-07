#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <optional>

#include <QString>
#include <QObject>

#include <grpcpp/grpcpp.h>

#include <CommonLib/HostAddressPort.h>
#include <CommonLib/Hash.h>
#include <CommonStdLib/TimesStd.h>
#include <GrpcAppDataSrv.grpc.pb.h>

#include "CircularLogger.h"
#include "SoftwareSettings.h"
#include "SoftwareInfo.h"
#include "TcpFileTransfer.h"

class GrpcClientQObject : public QObject
{
	Q_OBJECT

signals:
	void signal_unknownClientID(QString errMsg);
	void signal_wrongClientHostname(QString errMsg);
	void signal_connection();
	void signal_disconnection();
};

// -------------------------------------------------------------------------------------
//
// GrpcClient template class
//
// -------------------------------------------------------------------------------------

template <typename SERVICE_TYPE>
class GrpcClient : public GrpcClientQObject, public LogWrapper
{
protected:
	using Stub = typename SERVICE_TYPE::Stub;

public:
	GrpcClient(const SoftwareInfo& localSoftwareInfo,
			   const std::vector<HostAddressPort>& serverAddress,
			   const QString& clientDescription,
			   CircularLoggerShared log,
			   int pingPeriodMs)  :
		LogWrapper(log),
		m_localSwInfo(localSoftwareInfo),
		m_serverAddress(serverAddress),
		m_clientDescription(clientDescription)
	{
		setPingPeriod(pingPeriodMs);

		Q_ASSERT(m_serverAddress.size() > 0);

		m_state.localSoftwareInfo = localSoftwareInfo;
	}

	virtual ~GrpcClient()
	{
		Q_ASSERT(m_threadStarted.load(std::memory_order::acquire) == false);		// client must be stopped!
	}

	const SoftwareInfo& localSwInfo() const
	{
		return m_localSwInfo;
	}

	QString clientDescription() const
	{
		return m_clientDescription;
	}

	std::string authToken() const
	{
		std::lock_guard lg(m_stateMutex);
		return m_authToken;
	}

	void setAuthToken(const std::string& token)
	{
		std::lock_guard lg(m_stateMutex);
		m_authToken = token;
	}

	void clearAuthToken()
	{
		std::lock_guard lg(m_stateMutex);
		m_authToken.clear();
	}

	void start()
	{
		if (m_serverAddress.size() == 0)
		{
			Q_ASSERT(false);
			return;
		}

		if (m_threadStarted.load(std::memory_order::acquire))
		{
			Q_ASSERT(false);          // уже запущен
			return;
		}

		m_quitRequested.store(false, std::memory_order::relaxed);
		m_thread = std::thread(&GrpcClient::run, this);
		m_threadStarted.store(true, std::memory_order::release);

		qDebug() << C_STR(QString("%1 started").arg(m_clientDescription));
	}

	void stop()
	{
		if (m_threadStarted.exchange(false, std::memory_order::acquire) == false)
		{
			return;
		}

		m_quitRequested.store(true, std::memory_order::relaxed);

		wakeupThread();

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		m_threadStarted.store(false, std::memory_order::release);

		qDebug() << C_STR(QString("%1 stoped").arg(m_clientDescription));
	}

	bool isThreadStarted() const { return m_threadStarted.load(std::memory_order::acquire); }
	bool isQuitRequested() const { return m_quitRequested.load(std::memory_order::relaxed); }

	HostAddressPort getConnectedServerAddr() const
	{
		std::lock_guard lg(m_stateMutex);
		return m_state.peerAddr;
	}

	Tcp::ConnectionState getConnectionState() const
	{
		std::lock_guard lg(m_stateMutex);
		return m_state;
	}

	void setConnectionState(const Tcp::ConnectionState& state)
	{
		std::lock_guard lg(m_stateMutex);
		m_state = state;
	}

	bool isConnected() const
	{
		std::lock_guard lg(m_stateMutex);
		return m_state.isConnected;
	}

	int selectedServerIndex() const { return m_srvAddrIndex; }

	Stub* stub() { return m_stub.get(); }
	const Stub* stub() const { return m_stub.get(); }

	void resetStub()
	{
		m_stub.reset();
		m_authToken.clear();
		adsDisconnected();
		emit signal_disconnection();
	}

	bool createContext(grpc::ClientContext* ctx, int deadlineMs = 1000)
	{
		TEST_PTR_RETURN_FALSE(ctx)

		if (stub() == nullptr || authToken().empty())
		{
			return false;
		}

		ctx->AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken());
		ctx->set_deadline(makeDeadlineMs(deadlineMs));

		return true;
	}

	void setPingPeriod(int timeoutMs)
	{
		static constexpr int MIN_PING_TIMEOUT = 500;

		timeoutMs = std::max(timeoutMs, MIN_PING_TIMEOUT);

		m_pingPeriodMs.store(timeoutMs);
	}

	int pingPeriod() const
	{
		return m_pingPeriodMs.load(std::memory_order_relaxed);
	}

protected:
	std::string getNextServerAddr() const
	{
		const int size = TO_INT(m_serverAddress.size());
		Q_ASSERT(size > 0);

		const int idx =	(m_srvAddrIndex.fetch_add(1, std::memory_order_relaxed) + 1) % size;

		return m_serverAddress[idx].addressPortStr().toStdString();
	}

	virtual void run()
	{
		while(isQuitRequested() == false)
		{
			if (authToken().empty())
			{
				createStubAndHandshake();

				if (authToken().empty())
				{
					waitForOrQuit(500);
					continue;
				}
			}

			// add processing here
			processing();
		}

		resetStub();
	}

	virtual void processing()
	{
		waitForOrQuit(200);
	}

	virtual void wakeupThread()
	{
		m_processigCondition.notify_all();
	}

	bool waitForOrQuit(const int64_t timeoutMs)
	{
		if (isQuitRequested())
		{
			return false;
		}

		std::chrono::milliseconds timeout(timeoutMs);

		std::unique_lock<std::mutex> ul(m_processingMutex);

		m_processigCondition.wait_for(ul, timeout, [this]()
							{
								return isQuitRequested();
							});

		return !isQuitRequested();
	}

	bool waitUntilOrQuit(const int64_t untilTimeMs)
	{
		if (isQuitRequested())
		{
			return false;
		}

		auto timeout = std::chrono::system_clock::time_point(std::chrono::milliseconds(untilTimeMs));

		std::unique_lock<std::mutex> ul(m_processingMutex);

		m_processigCondition.wait_until(ul, timeout, [this]()
									  {
										  return isQuitRequested();
									  });

		return !isQuitRequested();
	}

	virtual void adsConnected()
	{
	}

	virtual void adsDisconnected()
	{
	}

	virtual void createStubAndHandshake(grpc::Status* status = nullptr)
	{
		logMsg(QString("%1::createStubAndHandshake").arg(clientDescription()));

		const std::string endpoint = getNextServerAddr();

		auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
		m_stub = SERVICE_TYPE::NewStub(channel);

		grpc::ClientContext handshakeCtx;

		handshakeCtx.set_deadline(makeDeadlineMs(1000));

		Grpc::HandshakeRequest req;
		Grpc::HandshakeReply rep;

		localSwInfo().serializeTo(req.mutable_clientsoftwareinfo());

		grpc::Status st = m_stub->Handshake(&handshakeCtx, req, &rep);

		if (status != nullptr)
		{
			*status = st;
		}

		Tcp::ConnectionState state;

		if (st.ok())
		{
			setAuthToken(rep.authtoken());

			//

			state.isSocketConnected = true;
			state.isConnected = true;
			state.connectedSoftwareInfo.serializeFrom(rep.serversoftwareinfo());
			state.localSoftwareInfo = localSwInfo();
			state.securityLevel = E::SecurityLevel::Basic;	// !!!
			state.setConnectionResult = Tcp::SetConnectionResult::Ok;
			state.connectionNo = 1;
			state.serverEquipmentID = state.connectedSoftwareInfo.equipmentID();
			state.peerAddr = HostAddressPort(rep.serverip(), rep.serverport());
			state.startTime = currentMSecsUTC();
			state.sentBytes = 0;
			state.receivedBytes = 0;
			state.requestCount = 1;
			state.replyCount = 1;
			state.isActual = false;

			setConnectionState(state);

			logMsg(QString("%1::createStubAndHandshake - Handshake Ok").arg(clientDescription()));

			emit signal_connection();
			adsConnected();
			return;
		}

		if (st.error_code() == grpc::StatusCode::UNAUTHENTICATED)
		{
			if (st.error_message() == Grpc::WRONG_CLIENT_EQUIPMENT_ID)
			{
				state.setConnectionResult = Tcp::SetConnectionResult::UnknownClientID;
				emit signal_unknownClientID(QString::fromStdString(Grpc::WRONG_CLIENT_EQUIPMENT_ID));
			}
			else
			{
				if (st.error_message() == Grpc::WRONG_HOST_NAME)
				{
					state.setConnectionResult = Tcp::SetConnectionResult::WrongClientHostname;
					emit signal_wrongClientHostname(QString::fromStdString(Grpc::WRONG_HOST_NAME));
				}
				else
				{
					Q_ASSERT(false);
				}
			}
		}

		clearAuthToken();

		state.localSoftwareInfo = localSwInfo();

		setConnectionState(state);

		logErr(QString("%1::createStubAndHandshake - Handshake Failed").arg(clientDescription()));

		m_stub.reset();
	}

	bool sendPingRequest()
	{
		grpc::ClientContext ctx;

		if (createContext(&ctx) == false)
		{
			return false;
		}

		Grpc::PingRequest req;
		Grpc::PingReply rep;

		req.set_authtoken(authToken());

		grpc::Status st = m_stub->Ping(&ctx, req, &rep);

		bool result = (req.authtoken() == rep.authtoken());

		if (st.ok() == false || result == false)
		{
			logErr(QString("%1::sendPingRequest - error").arg(clientDescription()));
			return false;
		}

//		logErr(QString("%1::sendPingRequest - Ok").arg(clientDescription()));

		return result;
	}

	static std::chrono::system_clock::time_point makeDeadlineMs(int ms)
	{
		return std::chrono::system_clock::now()	+ std::chrono::milliseconds(ms);
	}

protected:
	std::mutex m_processingMutex;
	std::condition_variable m_processigCondition;

private:
	const SoftwareInfo m_localSwInfo;
	std::vector<HostAddressPort> m_serverAddress;
	QString m_clientDescription;
	std::atomic<int64_t> m_pingPeriodMs {5000};

	//

	std::thread m_thread;
	std::atomic_bool m_threadStarted {false};
	std::atomic_bool m_quitRequested {false};

	mutable std::atomic<int> m_srvAddrIndex{-1};

	//

	mutable std::mutex m_stateMutex;
	std::unique_ptr<Stub> m_stub;
	std::string m_authToken;
	Tcp::ConnectionState m_state;
};
