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
		virtual ~TuningLog() = default;

		virtual bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue);
		virtual bool write(const QString& message);

		virtual void viewSignalsLog(QWidget* parent);

	private:
		ClientLib::TuningUserManager& m_userManager;
	};


	class TuningLogStub : public TuningLog
	{
		Q_OBJECT

	public:
		TuningLogStub(ClientLib::TuningUserManager& userManager, const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 10);
		virtual ~TuningLogStub() = default;

		virtual bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue) override;
		virtual bool write(const QString& message) override;

		virtual void viewSignalsLog(QWidget* parent) override;
	};
}


