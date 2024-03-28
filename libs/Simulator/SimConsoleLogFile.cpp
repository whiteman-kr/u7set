#include "./include/Simulator/SimConsoleLogFile.h"

Q_LOGGING_CATEGORY(u7sim, "u7.sim")

namespace Sim
{

	bool ConsoleLogFile::writeAlert(const QString& text, const QString& /*tag = {}*/)
	{
		qCCritical(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeError(const QString& text, const QString& /*tag = {}*/)
	{
		qCCritical(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeWarning(const QString& text, const QString& /*tag = {}*/)
	{
		qCWarning(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeMessage(const QString& text, const QString& /*tag = {}*/)
	{
		qCInfo(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeText(const QString& text, const QString& /*tag = {}*/)
	{
		qCDebug(u7sim).noquote() << text;
		return true;
	}

} // namespace Sim
