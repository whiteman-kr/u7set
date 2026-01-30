#pragma once

#include <atomic>
#include <thread>
#include <string>
#include <iostream>

#include <QCoreApplication>
#include <QMetaObject>

inline std::atomic<bool> exitByPosixSignal { false };

#if defined(Q_OS_LINUX)

#include <signal.h>

class PosixSignalHandler
{
public:
	PosixSignalHandler() = delete;

	static void install() noexcept
	{
		// Use sigaction (preferred over signal()).
		//
		struct sigaction sa {};

		sa.sa_handler = &PosixSignalHandler::onSignal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;

		(void)::sigaction(SIGTERM, &sa, nullptr);
		(void)::sigaction(SIGINT, &sa, nullptr);
	}

private:
	static void onSignal(int signum) noexcept
	{
		(void)signum;

		exitByPosixSignal.store(true, std::memory_order_relaxed);
	}
};

#endif		// Q_OS_LINUX

#if defined(Q_OS_LINUX)
	#include <unistd.h>
	#include <sys/select.h>
	#include <errno.h>
#elif defined(Q_OS_WIN)
	#include <windows.h>
#endif

class WaitSignalOrKbInputThread
{
public:
	WaitSignalOrKbInputThread() = default;

	~WaitSignalOrKbInputThread()
	{
		stop();
	}

	void start()
	{
		m_running.store(true, std::memory_order_relaxed);
		m_thread = std::thread(&WaitSignalOrKbInputThread::run, this);
	}

	void stop()
	{
		m_running.store(false, std::memory_order_relaxed);

		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

private:
	static void requestQtExit()
	{
		if (qApp == nullptr)
		{
			return;
		}

		QMetaObject::invokeMethod(
			qApp,
			[]()
			{
				QCoreApplication::exit(0);
			},
			Qt::QueuedConnection);
	}

	void run()
	{
		std::string line;

#if defined(Q_OS_LINUX)

		while (m_running.load(std::memory_order_relaxed))
		{
			if (exitByPosixSignal.load(std::memory_order_relaxed))
			{
				requestQtExit();
				return;
			}

			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);

			timeval tv {};
			tv.tv_sec = 0;
			tv.tv_usec = 200000; // 200 ms

			const int res = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

			if (!m_running.load(std::memory_order_relaxed))
			{
				return;
			}

			if (res > 0 && FD_ISSET(STDIN_FILENO, &fds))
			{
				char ch = 0;
				const ssize_t n = ::read(STDIN_FILENO, &ch, 1);

				if (n > 0)
				{
					requestQtExit();
				}

				return;
			}
			else if (res < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}

				return;
			}
		}

#elif defined(Q_OS_WIN)

		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

		if (hStdin == INVALID_HANDLE_VALUE || hStdin == nullptr)
		{
			return;
		}

		DWORD oldMode = 0;
		const bool modeOk = (GetConsoleMode(hStdin, &oldMode) != 0);

		if (!modeOk)
		{
			return;
		}

		struct ConsoleModeGuard
		{
			HANDLE h = nullptr;
			DWORD mode = 0;

			~ConsoleModeGuard()
			{
				SetConsoleMode(h, mode);
			}
		} guard { hStdin, oldMode };

		DWORD newMode = oldMode;
		newMode |= ENABLE_ECHO_INPUT;
		newMode |= ENABLE_LINE_INPUT;
		newMode |= ENABLE_PROCESSED_INPUT;
		SetConsoleMode(hStdin, newMode);

		wchar_t buf[512] {};
		DWORD readCount = 0;

		while (m_running.load(std::memory_order_relaxed))
		{
			const DWORD waitRes = WaitForSingleObject(hStdin, 200);

			if (!m_running.load(std::memory_order_relaxed))
			{
				return;
			}

			if (waitRes == WAIT_OBJECT_0)
			{
				if (ReadConsoleW(hStdin, buf, static_cast<DWORD>(std::size(buf) - 1), &readCount, nullptr))
				{
					requestQtExit();
				}

				return;
			}
			else if (waitRes == WAIT_FAILED)
			{
				return;
			}
		}

#else

		if (std::getline(std::cin, line))
		{
			requestQtExit();
		}

#endif
	}

private:
	std::thread m_thread;
	std::atomic<bool> m_running { false };
};
