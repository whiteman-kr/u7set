#include "TestLog.h"

namespace TestSuite
{
	TestLogItem::TestLogItem(int no, const QString& text, TestLogItemType type)	:
		m_no(no),
		m_message(text),
		m_type(type),
		m_dateTime(QDateTime::currentDateTime())
	{
	}

	QString TestLogItem::toText() const
	{
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

		return QObject::tr("%1 | %2 | %3 | %4")
				.arg(m_no, 4, 10, QChar('0'))
				.arg(m_dateTime.toString("hh:mm:ss:zzz"))
				.arg(type)
				.arg(m_message);
	}

	QString TestLogItem::toHtml() const
	{
		QString result;

		if (m_message.isEmpty())
		{
			result = QString("<font face=\"%1\" size=\"4\" color=#C0C0C0>%2|</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'));

			return result;
		}

		switch (m_type)
		{
		case TestLogItemType::Message:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=black>%4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message);
			break;
		case TestLogItemType::Warning:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=#F87217>WRN %4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message);
			break;
		case TestLogItemType::Error:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=red>ERR %4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message);
			break;

		default:
			assert(false);
		}

		return result;
	}

	TestLogItemType TestLogItem::type() const
	{
		return m_type;
	}

	TestLog::TestLog(ITestLogOutput* logOutput):
		m_logOutput(logOutput)
	{
		Q_ASSERT(m_logOutput);
	}

	void TestLog::clear()
	{
		no = 0;

		QWriteLocker l(&m_itemsLock);
		m_items.clear();
	}

	void TestLog::writeError(const QString& text)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return;
		}

		TestLogItem ti(no++, text, TestLogItemType::Error);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);
	}

	void TestLog::writeWarning(const QString& text)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return;
		}

		TestLogItem ti(no++, text, TestLogItemType::Warning);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);
	}

	void TestLog::writeMessage(const QString& text)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return;
		}

		TestLogItem ti(no++, text, TestLogItemType::Message);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);
	}
}
