#include "AdsGatewayServer.h"

AdsGatewayServer::AdsGatewayServer(const HostAddressPort& listenIP, CircularLoggerShared log) :
	LogWrapper(log, "AdsGatewayServer"),
	m_listenIP(listenIP)
{
}

AdsGatewayServer::~AdsGatewayServer()
{
}

void AdsGatewayServer::run()
{
	bool expected = false;

	if (!m_running.compare_exchange_strong(expected, true))
	{
		Q_ASSERT(false);
		return;
	}

	m_serverThread = std::thread(
		[this]()
		{
			runAcceptLoop();
		});
}

void AdsGatewayServer::stop()
{
	if (!m_running.exchange(false))
	{
		return;
	}

	// cancel acceptor
	//
	{
		std::lock_guard<std::mutex> lock(m_acceptorMutex);

		if (m_acceptor)
		{
			asio::error_code ec;
			m_acceptor->cancel(ec);
			m_acceptor->close(ec);
		}
	}

	// close active sessions
	//
	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (auto& sock : m_sessionSockets)
		{
			if (sock)
			{
				asio::error_code ec;
				sock->shutdown(tcp::socket::shutdown_both, ec);
				sock->close(ec);
			}
		}
	}

	if (m_serverThread.joinable())
	{
		m_serverThread.join();
	}

	// wait while all sessions stopped
	//
	joinAllSessions();
}

void AdsGatewayServer::runAcceptLoop()
{
	try
	{
		asio::io_context io;

		tcp::endpoint ep(asio::ip::address_v4::from_string(m_listenIP.addressStr().toStdString()), m_listenIP.port());
		tcp::acceptor acceptor(io);

		acceptor.open(ep.protocol());
		acceptor.set_option(tcp::acceptor::reuse_address(true));
		acceptor.bind(ep);
		acceptor.listen();

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor = &acceptor;
		}

		logMsg(QString("starts listening %1").arg(m_listenIP.addressPortStr()));

		while (m_running.load())
		{
			reapFinishedSessions();

			tcp::socket socket(io);
			asio::error_code ec;

			acceptor.accept(socket, ec);

			if (!m_running.load())
			{
				break;
			}

			if (ec)
			{
				continue;
			}

			auto sockPtr = std::make_shared<tcp::socket>(std::move(socket));

			logMsg(QString("accept new connectio from %1").arg(getIpPortStr(sockPtr)));

			{
				std::lock_guard<std::mutex> lock(m_sessionsMutex);
				m_sessionSockets.push_back(sockPtr);
			}

			// start session thread
			//
			auto finished = std::make_shared<std::atomic<bool>>(false);

			{
				std::lock_guard<std::mutex> lock(m_threadsMutex);

				m_sessionThreads.push_back(
					SessionThread
					{
						std::thread(
							[this, sockPtr, finished]()
							{
								QString remoteIpPort = getIpPortStr(sockPtr);

								logMsg(QString("session thread for %1 started").arg(remoteIpPort));

								sessionThread(sockPtr);
								removeSessionSocket(sockPtr);
								finished->store(true);

								logMsg(QString("session thread for %1 finished").arg(remoteIpPort));
							}),
						finished
						});
					}
			}

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor = nullptr;
		}
	}
	catch (const std::exception& ex)
	{
		logErr(QString("accept loop error: %1").arg(ex.what()));
	}

	logMsg(QString("stops"));
}

void AdsGatewayServer::sessionThread(const std::shared_ptr<tcp::socket>& socket)
{
	static const std::size_t BUF_SIZE = 4096;

	try
	{
		asio::error_code ec;

		socket->set_option(tcp::no_delay(true), ec);

		std::array<char, BUF_SIZE> buf {};

		while (m_running.load())
		{
			std::size_t n = socket->read_some(asio::buffer(buf), ec);

			if (ec)
			{
				break;
			}

			// write() гарантирует, что запишет всё (или вернёт ошибку)
			asio::write(*socket, asio::buffer(buf.data(), n), ec);

			if (ec)
			{
				break;
			}
		}
	}
	catch (const std::exception& ex)
	{
		logErr(QString("session error: %1").arg(ex.what()));
	}

	{
		asio::error_code ec;
		socket->shutdown(tcp::socket::shutdown_both, ec);
		socket->close(ec);
	}
}

void AdsGatewayServer::removeSessionSocket(const std::shared_ptr<tcp::socket>& socket)
{
	std::lock_guard<std::mutex> lock(m_sessionsMutex);

	for (auto it = m_sessionSockets.begin(); it != m_sessionSockets.end(); ++it)
	{
		if (*it == socket)
		{
			m_sessionSockets.erase(it);
			break;
		}
	}
}

void AdsGatewayServer::reapFinishedSessions()
{
	std::lock_guard<std::mutex> lock(m_threadsMutex);

	for (std::size_t i = 0; i < m_sessionThreads.size();)
	{
		SessionThread& st = m_sessionThreads[i];

		if (st.finished && st.finished->load())
		{
			if (st.thread.joinable())
			{
				st.thread.join();
			}

			m_sessionThreads.erase(m_sessionThreads.begin() + static_cast<std::ptrdiff_t>(i));
			continue;
		}

		++i;
	}
}

void AdsGatewayServer::joinAllSessions()
{
	std::vector<SessionThread> threads;

	{
		std::lock_guard<std::mutex> lock(m_threadsMutex);
		threads.swap(m_sessionThreads);
	}

	for (auto& st : threads)
	{
		if (st.thread.joinable())
		{
			st.thread.join();
		}
	}
}

QString AdsGatewayServer::getIpPortStr(const std::shared_ptr<tcp::socket>& socket)
{
	if (socket == nullptr)
	{
		Q_ASSERT(false);
		return Separator::EMPTY_STR;
	}

	tcp::endpoint remote = socket->remote_endpoint();

	return QString("%1:%2").arg(QString::fromStdString(remote.address().to_string())).arg(remote.port());
}

