#pragma once

#include <asio.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <CommonLib/HostAddressPort.h>
#include <GatewayClientLib/GwClient.hpp>
#include <GatewayClientLib/GwCrc32.hpp>
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CircularLogger.h"
#include "../AppSignalLib/AppSignal.h"

namespace GCL = GatewayClientLib;

template<class SessionT>
class AsyncTcpServer;

class AsyncTcpSession : public std::enable_shared_from_this<AsyncTcpSession>, public LogWrapper
{
	using Buffer = std::vector<char>;

public:
	static constexpr size_t MIN_READ_BUFFER_SIZE = 1024;
	static constexpr size_t MAX_READ_BUFFER_SIZE = 4 * 1024 * 1024;

public:
	explicit AsyncTcpSession(const SoftwareInfo& swInfo,
							 const AppSignals& appSignals,
							 const std::vector<HostAddressPort>& serviceAddresses, 
							 asio::ip::tcp::socket socket, 
							 CircularLoggerShared log);

	virtual ~AsyncTcpSession();

	void setBufferSize(size_t size);

	QString remoteIpPortStr() const;

protected:
	virtual void onStarted();
	virtual void onStopped();

	virtual bool checkRequestID(uint32_t requestID) = 0;
	virtual bool isHandshakeRequest(uint32_t requestID) = 0;
	virtual bool checkPayloadSize(const GCL::GwMessageHeader& header,
								  const char* recvBuf,
								  const size_t recvBufSize,
								  GCL::GwErrorCode& errCode) = 0;

	virtual bool processRequest(const GCL::GwMessageHeader& header, char* recvBuf, size_t recvBufSize) = 0;

	virtual void onError(const std::error_code& ec);

	void sendErrReply(const GCL::GwMessageHeader& requestHeader, GCL::GwErrorCode errCode);
	void sendOkReply(const GCL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize);
	void sendReply(uint32_t requestID, GCL::GwErrorCode errCode, const char* payloadData, size_t payloadSize);

	[[nodiscard]] size_t skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize);

	bool isHandshakeCompleted() const;
	void setHandshakeCompleted(bool completed);

private:
	void start();
	void stop();

	void startReceive();
	void startSend();

	void onDataReceived(char* recvBuf, std::size_t recvBufSize);

private:
	SoftwareInfo m_swInfo;
	const AppSignals& m_appSignals;
	std::vector<HostAddressPort> m_serviceAdresses;

	asio::ip::tcp::socket m_socket;
	asio::strand<asio::any_io_executor> m_strand;

	bool m_started = false;

	Buffer m_recvBuf;
	size_t m_recvBufSize = 0;
	Buffer m_sendBuf;
	bool m_writeInProgress = false;
	quint64 m_errCount = 0;

	bool m_handshakeCompleted = false;

	template<class SessionT>
	friend class AsyncTcpServer;
};

template<class SessionT>
class AsyncTcpServer : public LogWrapper
{
public:
	explicit AsyncTcpServer(const SoftwareInfo& swInfo,
							const AppSignals& appSignals,
							const std::vector<HostAddressPort>& listenAddresses, 
							const std::vector<HostAddressPort>& serviceAddresses, 
							int threadsCount, 
							CircularLoggerShared log,
							const QString& description = "AsyncTcpServer") :
		LogWrapper(log, description),
		m_swInfo(swInfo),
		m_appSignals(appSignals),
		m_listenAddresses(listenAddresses),
		m_serviceAddresses(serviceAddresses),
		m_threadsCount(threadsCount),
		m_workGuard(asio::make_work_guard(m_ioContext))
	{
		if (m_threadsCount <= 0)
		{
			m_threadsCount = 1;
		}
	}

	~AsyncTcpServer() 
	{ 
		stop(); 
	}

	bool start()
	{
		if (m_running.exchange(true))
		{
			return true;
		}

		for (const HostAddressPort& listenAddress : m_listenAddresses)
		{
			if (!startAcceptor(listenAddress))
			{
				stop();
				return false;
			}
		}

		for (int i = 0; i < m_threadsCount; ++i)
		{
			m_threads.emplace_back(
				[this]()
				{
					m_ioContext.run();
				});
		}

		return true;
	}

	void stop()
	{
		if (!m_running.exchange(false))
		{
			return;
		}

		std::error_code ec;

		for (std::shared_ptr<asio::ip::tcp::acceptor>& acceptor : m_acceptors)
		{
			if (acceptor && acceptor->is_open())
			{
				acceptor->close(ec);
			}
		}

		m_workGuard.reset();
		m_ioContext.stop();

		for (std::thread& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}

private:
	bool startAcceptor(const HostAddressPort& listenAddress)
	{
		using asio::ip::tcp;

		std::error_code ec;

		asio::ip::address address = asio::ip::make_address(listenAddress.addressPortStr().toStdString(), ec);

		if (ec)
		{
			logErr(QString("Bad IP %1: %2").arg(listenAddress.addressPortStr()).arg(qstr(ec.message())));
			return false;
		}

		tcp::endpoint endpoint(address, listenAddress.port());

		auto acceptor = std::make_shared<tcp::acceptor>(m_ioContext);

		acceptor->open(endpoint.protocol(), ec);

		if (ec)
		{
			logErr(QString("Acceptor Open failed: %1").arg(qstr(ec.message())));
			return false;
		}

		acceptor->set_option(tcp::acceptor::reuse_address(true), ec);

		if (ec)
		{
			logErr(QString("Set reuse_address failed: %1").arg(qstr(ec.message())));
			return false;
		}

		acceptor->bind(endpoint, ec);

		if (ec)
		{
			logErr(QString("Bind failed %1: %2").arg(listenAddress.addressPortStr()).arg(qstr(ec.message())));
			return false;
		}

		acceptor->listen(asio::socket_base::max_listen_connections, ec);

		if (ec)
		{
			logErr(QString("Listen failed: %1").arg(qstr(ec.message())));
			return false;
		}

		m_acceptors.push_back(acceptor);

		accept(acceptor);

		logMsg(QString("Listening on %1").arg(listenAddress.addressPortStr()));

		return true;
	}

	void accept(std::shared_ptr<asio::ip::tcp::acceptor> acceptor)
	{
		acceptor->async_accept(
			[this, acceptor](const std::error_code& ec, asio::ip::tcp::socket socket)
			{
				if (!m_running.load())
				{
					return;
				}

				if (!acceptor->is_open())
				{
					return;
				}

				if (!ec)
				{
					std::make_shared<SessionT>(m_swInfo, m_appSignals, m_serviceAddresses, std::move(socket), getLog())->start();
				}
				else
				{
					logErr(QString("Accept error: %1").arg(qstr(ec.message())));
				}

				accept(acceptor);
			});
	}

private:
	SoftwareInfo m_swInfo;
	const AppSignals& m_appSignals;
	std::vector<HostAddressPort> m_listenAddresses;
	std::vector<HostAddressPort> m_serviceAddresses;

	asio::io_context m_ioContext;
	std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> m_acceptors;
	asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;

	int m_threadsCount = 1;
	std::vector<std::thread> m_threads;

	std::atomic_bool m_running = false;
};