#pragma once

#include "../UtilsLib/LogFile.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "TuningUserManager.h"

namespace TuningLog
{
	class TuningLog : public Log::LogFile
	{
		Q_OBJECT

	public:
		TuningLog(ClientLib::TuningUserManager& userManager, const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 10);
		virtual ~TuningLog();

		bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue);
		bool write(const QString& message);

		void viewSignalsLog(QWidget* parent);

	private:
		ClientLib::TuningUserManager& m_userManager;
	};
}


