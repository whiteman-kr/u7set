#include "SimConsoleLogFile.h"
#include <QtGlobal>

Q_LOGGING_CATEGORY(u7sim, "u7.sim")

namespace Sim
{

	bool ConsoleLogFile::writeAlert(const QString& text)
	{
		qCCritical(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeError(const QString& text)
	{
		qCCritical(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeWarning(const QString& text)
	{
		qCWarning(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeMessage(const QString& text)
	{
		qCInfo(u7sim).noquote() << text;
		return true;
	}

	bool ConsoleLogFile::writeText(const QString& text)
	{
		qCDebug(u7sim).noquote() << text;
		return true;
	}

}
