#pragma once

#include "../ClientLib/ITuningLog.h"
#include "../UtilsLib/LogFile.h"
#include "TuningUserManager.h"

namespace ClientLib
{
	class TuningLog : public ITuningLog
	{
	public:
		TuningLog(ITuningAuthorization& m_tuningAuthorization, const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 10);

		virtual bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue) override;
		virtual bool write(const QString& message) override;

		virtual void viewTuningLog(QWidget* parent) override;

	private:
		Log::LogFile m_logFile;
		ITuningAuthorization& m_tuningAuthorization;
	};
}
