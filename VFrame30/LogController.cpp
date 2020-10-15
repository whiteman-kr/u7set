#include "LogController.h"

namespace VFrame30
{
	LogController::LogController(ILogFile* logFile, QObject* parent)
		:QObject(parent),
		  m_logFile(logFile)
	{
		Q_ASSERT(logFile);
		return;
	}

	bool LogController::writeAlert(QString text)
	{
		if (m_logFile == nullptr)
		{
			qDebug() << "Warning: Attempt to write log while m_logFile is not set.";
			return false;
		}

		return m_logFile->writeAlert(text);
	}

	bool LogController::writeError(QString text)
	{
		if (m_logFile == nullptr)
		{
			qDebug() << "Warning: Attempt to write log while m_logFile is not set.";
			return false;
		}

		return m_logFile->writeError(text);
	}

	bool LogController::writeWarning(QString text)
	{
		if (m_logFile == nullptr)
		{
			qDebug() << "Warning: Attempt to write log while m_logFile is not set.";
			return false;
		}

		return m_logFile->writeWarning(text);
	}

	bool LogController::writeMessage(QString text)
	{
		if (m_logFile == nullptr)
		{
			qDebug() << "Warning: Attempt to write log while m_logFile is not set.";
			return false;
		}

		return m_logFile->writeMessage(text);
	}

	bool LogController::writeText(QString text)
	{
		if (m_logFile == nullptr)
		{
			qDebug() << "Warning: Attempt to write log while m_logFile is not set.";
			return false;
		}

		return m_logFile->writeText(text);
	}
}
