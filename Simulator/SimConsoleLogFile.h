#pragma once
#include "../UtilsLib/ILogFile.h"

Q_DECLARE_LOGGING_CATEGORY(u7sim)

namespace Sim
{

	class ConsoleLogFile : public ILogFile
	{
	public:
		virtual bool writeAlert(const QString& text) override;
		virtual bool writeError(const QString& text) override;
		virtual bool writeWarning(const QString& text) override;
		virtual bool writeMessage(const QString& text) override;
		virtual bool writeText(const QString& text) override;
	};

}

