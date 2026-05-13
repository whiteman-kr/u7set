#pragma once

#include <asio.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <CommonLib/HostAddressPort.h>
#include "../OnlineLib/CircularLogger.h"

template<class SessionT>
class AsyncTcpServer;

class AsyncTcpSession : public std::enable_shared_from_this<AsyncTcpSession>, public LogWrapper
{
	using Buffer = std::vector<char>;

public:
	static constexpr size_t MIN_READ_BUFFER_SIZE = 1024;
	static constexpr size_t MAX_READ_BUFFER_SIZE = 4 * 1024 * 1024;

public:
	explicit AsyncTcpSession(asio::ip::tcp::socket socket, CircularLoggerShared log);
	virtual ~AsyncTcpSession();

	void setBufferSize(size_t size);

	QString remoteIpPortStr() const;

protected:
	virtual void onStarted();
	virtual void onStopped();

	virtual void onDataReceived(const char* data, std::size_t size) = 0;
	virtual void onError(const std::error_code& ec);

	void send(const std::vector<char>& data);
	void send(const char* data, size_t dataSize);

private:
	void start();
	void stop();

	void startRead();
	void startWrite();

private:
	asio::ip::tcp::socket m_socket;
	asio::strand<asio::any_io_executor> m_strand;

	bool m_started = false;

	Buffer m_readBuffer;
	Buffer m_writeBuffer;
	bool m_writeInProgress = false;

	template<class SessionT>
	friend class AsyncTcpServer;
};

template<class SessionT>
class AsyncTcpServer : public LogWrapper
{
public:
	explicit AsyncTcpServer(const std::vector<HostAddressPort>& addresses, 
							int threadsCount, 
							CircularLoggerShared log,
							const QString& description = "AsyncTcpServer") :
		LogWrapper(log, description),
		m_addresses(addresses),
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

		for (const HostAddressPort& addressPort : m_addresses)
		{
			if (!startAcceptor(addressPort))
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
	bool startAcceptor(const HostAddressPort& addressPort)
	{
		using asio::ip::tcp;

		std::error_code ec;

		asio::ip::address address = asio::ip::make_address(addressPort.addressPortStr.toStdString(), ec);

		if (ec)
		{
			logErr(QString("Bad IP %1: %2").arg(addressPort.addressPortStr()).arg(qstr(ec.message())));
			return false;
		}

		tcp::endpoint endpoint(address, addressPort.port);

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
			logErr(QString("Bind failed %1: %2").arg(addressPort.addressPortStr()).arg(qstr(ec.message())));
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

		logMsg(QString("Listening on %1").arg(addressPort.addressPortStr()));

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
					std::make_shared<SessionT>(std::move(socket), getLog())->start();
				}
				else
				{
					logErr(QString("Accept error: %1").arg(qstr(ec.message())));
				}

				accept(acceptor);
			});
	}

private:
	asio::io_context m_ioContext;
	asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;

	std::vector<HostAddressPort> m_addresses;
	std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> m_acceptors;

	int m_threadsCount = 1;
	std::vector<std::thread> m_threads;

	std::atomic_bool m_running = false;
};