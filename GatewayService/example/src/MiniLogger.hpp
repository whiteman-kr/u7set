#pragma once

#include <format>
#include <iostream>
#include <string_view>

namespace adsgw
{
	class IMiniLogger
	{
	public:
		virtual ~IMiniLogger() = default;

		virtual void logTrace(std::string_view message) = 0;
		virtual void logWarning(std::string_view message) = 0;
		virtual void logError(std::string_view message) = 0;

		virtual bool isTraceEnabled() const = 0;

		template<typename... Args>
		decltype(auto) logTraceFormat(std::format_string<Args...> fmt, Args&&... args)
		{
			if (isTraceEnabled() == true)
			{
				logTrace(std::format(fmt, std::forward<Args>(args)...));
			}
		}
	};

	class ConsoleMiniLogger : public IMiniLogger
	{
	public:
		void logTrace(std::string_view message) override
		{
			if (isTraceEnabled() == true)
			{
				std::cout << "[TRACE] " << message << std::endl;
			}
		}
		void logWarning(std::string_view message) override { std::cout << "[WARNING] " << message << std::endl; }
		void logError(std::string_view message) override { std::cerr << "[ERROR] " << message << std::endl; }

		void setTraceEnabled(bool enabled) { m_traceEnabled = enabled; }
		bool isTraceEnabled() const override { return m_traceEnabled; }

	private:
		bool m_traceEnabled{false};
	};

} // namespace adsgw