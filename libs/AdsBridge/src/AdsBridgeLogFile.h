#pragma once
#include <AdsBridge/Common.h>
#include <AdsConnectionLib/ILoggerStd.h>

namespace AdsBridge
{
	class LogFile : public ILoggerStd
	{
	public:
		virtual void writeAlert(std::string_view message) override;
		virtual void writeError(std::string_view message) override;
		virtual void writeWarning(std::string_view message) override;
		virtual void writeMessage(std::string_view message) override;

	private:
		void privateLog(MatsLogLevel level, const char* message);

	public:
		static MatsLogHandler g_logHandler;
		static MatsLogLevel g_logLevel;
	};
} // namespace AdsBridge