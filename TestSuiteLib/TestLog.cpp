#include "TestLog.h"
#include "../UtilsLib/CsvFile.h"

namespace TestSuite
{
	TestLogItem::TestLogItem(int no, TestLogItemType type, const QString& text, const QString& tag):
		m_no(no),
		m_dateTime(QDateTime::currentDateTime()),
		m_type(type),
		m_message(text),
		m_tag(tag)
	{
	}

	QString TestLogItem::toText() const
	{
		QString type;
		switch(m_type)
		{
		case TestLogItemType::Text:
			return m_message;
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
		case TestLogItemType::Text:
			result = QString("<font face=\"%1\" size=\"4\" color=black>%2</font>")
					 .arg(m_htmlFont)
					 .arg(m_message.toHtmlEscaped());
			break;
		case TestLogItemType::Message:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=black>%4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message.toHtmlEscaped());
			break;
		case TestLogItemType::Warning:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=#F87217>WRN %4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message.toHtmlEscaped());
			break;
		case TestLogItemType::Error:
			result = QString("<font face=\"%1\" size=\"4\" color=#808080>%2| %3  </font>"
							 "<font face=\"%1\" size=\"4\" color=red>ERR %4</font>")
					 .arg(m_htmlFont)
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_dateTime.toString("hh:mm:ss:zzz   "))
					 .arg(m_message.toHtmlEscaped());
			break;

		default:
			assert(false);
		}

		return result;
	}

	QStringList TestLogItem::toStringList() const
	{
		QStringList result;

		result << QString::number(CsvVersion);
		result << QString::number(m_no);
		result << m_dateTime.toString("hh:mm:ss:zzz");

		switch(m_type)
		{
		case TestLogItemType::Text:
			result << "";
			break;
		case TestLogItemType::Message:
			result << "MSG";
			break;
		case TestLogItemType::Warning:
			result << "WRN";
			break;
		case TestLogItemType::Error:
			result << "ERR";
			break;
		default:
			result << "???";
			Q_ASSERT(false);
		}

		result << m_message;
		result << m_tag;

		return result;
	}

	TestLogItem TestLogItem::fromStringList(const QString& str, bool* ok)
	{
		TestLogItem result;
		QStringList strings = CsvFile::csvToStrings(str);

		static TestLogItem err;

		if (strings.isEmpty() == true)
		{
			// No data
			//
			*ok = false;
			return err;
		}

		int version = strings[0].toInt();
		if (version != 1 || strings.size() != 6)
		{
			// Unsupported version
			*ok = false;
			return err;
		}

		result.m_no = strings[1].toInt();
		result.m_dateTime = QDateTime::fromString(strings[2], "hh:mm:ss:zzz");

		result.m_type = TestLogItemType::Text;
		if (strings[3] == "MSG")
			result.m_type = TestLogItemType::Message;
		else
			if (strings[3] == "WRN")
				result.m_type = TestLogItemType::Warning;
			else
				if (strings[3] == "ERR")
					result.m_type = TestLogItemType::Error;

		result.m_message = strings[4];
		result.m_tag = strings[5];

		*ok = true;
		return result;
	}

	const QDateTime& TestLogItem::dateTime() const
	{
		return m_dateTime;
	}

	const QString& TestLogItem::message() const
	{
		return m_message;
	}

	const QString& TestLogItem::tag() const
	{
		return m_tag;
	}

	TestLogItemType TestLogItem::type() const
	{
		return m_type;
	}

	bool TestLogItem::isError() const
	{
		return m_type == TestLogItemType::Error;
	}
	bool TestLogItem::isWarning() const
	{
		return m_type == TestLogItemType::Warning;
	}
	bool TestLogItem::isMessage() const
	{
		return m_type == TestLogItemType::Message;
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

	bool TestLog::empty() const
	{
		QWriteLocker l(&m_itemsLock);
		return m_items.empty();
	}

	bool TestLog::writeAlert(const QString& text, const QString& tag)
	{
		return writeError(text, tag);
	}

	bool TestLog::writeError(const QString& text, const QString& tag)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return false;
		}

		TestLogItem ti(no++, TestLogItemType::Error, text, tag);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);

		return true;
	}

	bool TestLog::writeWarning(const QString& text, const QString& tag)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return false;
		}

		TestLogItem ti(no++, TestLogItemType::Warning, text, tag);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);
		
		return true;
	}

	bool TestLog::writeMessage(const QString& text, const QString& tag)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return false;
		}

		TestLogItem ti(no++, TestLogItemType::Message, text, tag);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);

		return true;
	}

	bool TestLog::writeText(const QString& text, const QString& tag)
	{
		if (m_logOutput == nullptr)
		{
			Q_ASSERT(m_logOutput);
			return false;
		}

		TestLogItem ti(-1, TestLogItemType::Text, text, tag);
		m_logOutput->logItemArrived(ti);

		QWriteLocker l(&m_itemsLock);
		m_items.push_back(ti);

		return true;
	}

	std::vector<TestLogItem> TestLog::items() const
	{
		QReadLocker l(&m_itemsLock);
		return m_items;
	}

	bool TestLog::saveToCSV(const QString& fileName, QString* errorMsg) const
	{
		{
			QReadLocker l(&m_itemsLock);
			if (m_items.empty() == true)
			{
				return false;
			}
		}

		std::vector<TestLogItem> storeItems;

		{
			QReadLocker l(&m_itemsLock);
			storeItems.reserve(m_items.size());
			storeItems = m_items;
		}

		QFile file(fileName);
		bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
		if (ok == false)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QObject::tr("Cannot open file %1 for writing.").arg(fileName);
			}
			return false;
		}

		QTextStream out(&file);

		for (const TestLogItem& item : storeItems)
		{
			out << CsvFile::stringsToCSV(item.toStringList()).toUtf8() << Qt::endl;
		}

		return true;

	}

	bool TestLog::loadFromCSV(const QString& fileName, QString* errorMsg)
	{
		QFile file(fileName);
		bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
		if (ok == false)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QObject::tr("Cannot open file %1 for reading.").arg(fileName);
			}
			return false;
		}

		QTextStream in(&file);

		std::vector<TestLogItem> restoreItems;

		int lineCounter = 0;

		while (in.atEnd() == false)
		{
			lineCounter++;

			ok = false;
			TestLogItem item = TestLogItem::fromStringList(in.readLine(), &ok);

			if (ok == false)
			{
				if (errorMsg != nullptr)
				{
					*errorMsg = QObject::tr("Cannot read data from line %1 of the file %2.").arg(fileName).arg(lineCounter);
				}
				return false;
			}

			restoreItems.push_back(item);
		}

		{
			QWriteLocker l(&m_itemsLock);
			m_items = std::move(restoreItems);
		}

		return true;
	}
}
