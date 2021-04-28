#pragma once

#include "../../UtilsLib/LogFile.h"
#include "../../AppSignalLib/AppSignalParam.h"

namespace TuningLog
{
	class TuningLog : public Log::LogFile
	{
		Q_OBJECT
	public:
		TuningLog(const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 10);
		virtual ~TuningLog();

		bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue, const QString& userName);
		bool write(const QString& message, const QString& userName);

		void viewSignalsLog(QWidget* parent);
	};
}


