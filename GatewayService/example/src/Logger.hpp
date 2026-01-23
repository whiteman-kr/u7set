#pragma once

#include <atomic>
#include <format>
#include <iostream>
#include <string_view>
#include <syncstream>


namespace AdsGatewayLib
{
	class ILogger
	{
	public:
		virtual ~ILogger() = default;

		virtual void logTrace(std::string_view message) = 0;
		virtual void logMessage(std::string_view message) = 0;
		virtual void logWarning(std::string_view message) = 0;
		virtual void logError(std::string_view message) = 0;

		virtual bool isTraceEnabled() const = 0;

		template<typename... Args>
		void logTraceFormat(std::format_string<Args...> fmt, Args&&... args)
		{
			if (isTraceEnabled() == true)
			{
				logTrace(std::format(fmt, std::forward<Args>(args)...));
			}
		}
	};

	class ConsoleLogger : public ILogger
	{
	public:
		void logTrace(std::string_view message) override
		{
			if (isTraceEnabled() == true)
			{
				std::osyncstream(std::cout) << "[TRC] " << message << std::endl;
			}
		}
		void logMessage(std::string_view message) override { std::osyncstream(std::cout) << "[MSG] " << message << std::endl; }
		void logWarning(std::string_view message) override { std::osyncstream(std::cout) << "[WRN] " << message << std::endl; }
		void logError(std::string_view message) override { std::osyncstream(std::cerr) << "[ERR] " << message << std::endl; }

		void setTraceEnabled(bool enabled) { m_traceEnabled.store(enabled); }
		bool isTraceEnabled() const override { return m_traceEnabled.load(std::memory_order_relaxed); }

	private:
		std::atomic<bool> m_traceEnabled{false};
	};

} // namespace AdsGatewayLib