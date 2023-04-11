#include "TestLog.h"

namespace TestSuite
{
	TestLogItem::TestLogItem(const QString& text, TestLogItemType type)	:
		m_message(text),
		m_type(type),
		m_dateTime(QDateTime::currentDateTime())
	{
	}

	QString TestLogItem::toText() const
	{
		QString time = m_dateTime.toString("dd.MM.yyyy hh:mm:ss");

		QString type;
		switch(m_type)
		{
		case TestLogItemType::Message:
			type = "MSG";
			break;
		case TestLogItemType::Warning:
			type = "WRN";
			break;
		case TestLogItemType::Error:
			type = "ERR";
			break;
		default:
			type = "???";
			Q_ASSERT(false);
		}

		return QObject::tr("%1 [%2] %3").arg(time).arg(type).arg(m_message);
	}

	TestLog::TestLog(ITestLog* outputLog):
		m_outputLog(outputLog)
	{
		Q_ASSERT(m_outputLog);
	}

	void TestLog::writeError(const QString& text)
	{
		if (m_outputLog == nullptr)
		{
			Q_ASSERT(m_outputLog);
			return;
		}

		TestLogItem ti(text, TestLogItemType::Error);
		m_outputLog->writeError(ti.toText());

		QMutexLocker l(&m_itemsMutex);
		m_items.push_back(ti);
	}

	void TestLog::writeWarning(const QString& text)
	{
		if (m_outputLog == nullptr)
		{
			Q_ASSERT(m_outputLog);
			return;
		}

		TestLogItem ti(text, TestLogItemType::Warning);
		m_outputLog->writeWarning(ti.toText());

		QMutexLocker l(&m_itemsMutex);
		m_items.push_back(ti);
	}

	void TestLog::writeMessage(const QString& text)
	{
		if (m_outputLog == nullptr)
		{
			Q_ASSERT(m_outputLog);
			return;
		}

		TestLogItem ti(text, TestLogItemType::Message);
		m_outputLog->writeMessage(ti.toText());

		QMutexLocker l(&m_itemsMutex);
		m_items.push_back(ti);
	}
}
