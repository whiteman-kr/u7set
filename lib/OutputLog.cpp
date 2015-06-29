#include "../include/OutputLog.h"



OutputLogItem::OutputLogItem(int messageNo) :
	m_no(messageNo),
	m_level(OutputMessageLevel::Message)
{
}

OutputLogItem::OutputLogItem(int messageNo,
							 QString message,
			  OutputMessageLevel level,
			  QDateTime time,
			  QString file,
			  int fileLine,
			  QString func) :
	m_no(messageNo),
	m_message(message),
	m_level(level),
	m_time(time),
	m_file(file),
	m_fileLine(fileLine),
	m_func(func)
{
}

QString OutputLogItem::toText() const
{
	QString result;
	return result;
}

QString OutputLogItem::toHtml() const
{
	QString result;

	QString color;
	switch (m_level)
	{
	case OutputMessageLevel::Message:
		color = "black";
		break;
	case OutputMessageLevel::Success:
		color = "green";
		break;
	case OutputMessageLevel::Warning:
		color = "darkorange";
		break;
	case OutputMessageLevel::Error:
		color = "red";
		break;

	default:
		assert(false);
		color = "black";
	}

	if (m_message.isEmpty())
	{
		result = QString("<font face=\"Courier\" color=#C0C0C0>%1|</font>")
					 .arg(m_no, 4, 10, QChar('0'));
	}
	else
	{
		result = QString("<font face=\"Courier\" color=#808080>%1| %2  </font>"
						 "<font face=\"Courier\" color=%3>%4</font>")
					 .arg(m_no, 4, 10, QChar('0'))
					 .arg(m_time.toString("hh:mm:ss:zzz   "))
					 .arg(color)
					 .arg(m_message);
	}

	return result;
}

QString OutputLogItem::toCsv() const
{
	QString result;

	QString level;
	switch (m_level)
	{
	case OutputMessageLevel::Message:
		level = "MSG";
		break;
	case OutputMessageLevel::Success:
		level = "SCS";
		break;
	case OutputMessageLevel::Warning:
		level = "WRN";
		break;
	case OutputMessageLevel::Error:
		level = "ERR";
		break;

	default:
		assert(false);
		level = "UNK";
	}

	result = QString("%1; %2; %3; %4; %5; %6; %7")
				.arg(m_no, 4, 10, QChar('0'))
				.arg(m_time.toString("hh:mm:ss:zzz"))
				.arg(level)
				.arg(m_message)
				.arg(m_file)
				.arg(m_fileLine)
				.arg(m_func);

	return result;
}


// OutputLog
//

OutputLog::OutputLog(void)
{
}


OutputLog::~OutputLog(void)
{
}

void OutputLog::clear()
{
	{
		QMutexLocker locker(&m_mutex);

		m_messages.clear();
		m_warningCount = 0;
		m_errorCount = 0;
		m_messagesNo = 0;

		m_strLogging = false;
		m_strFullLog.clear();
	}

	emit messageLogCleared();

	return;
}

void OutputLog::write(const QString& str, OutputMessageLevel level, QString file, int fileLine, QString func)
{
	QDateTime time = QDateTime::currentDateTime();

	// Add data ato queue
	//
	switch (level)
	{
		case OutputMessageLevel::Warning:
			incWarningCount();
			break;
		case OutputMessageLevel::Error:
			incErrorCount();
			break;
		default:
			break;
	}

	QMutexLocker locker(&m_mutex);
	OutputLogItem li(m_messagesNo ++, str, level, time, file, fileLine, func);
	m_messages.push_back(li);

	if (m_strLogging == true)
	{
		QString logText = li.toCsv();

		m_strFullLog.append(logText);
		m_strFullLog.append('\n');
	}

	locker.unlock();

	return;
}

void OutputLog::writeMessage(const QString& str)
{
	return write(str, OutputMessageLevel::Message, QString(""), 0, QString(""));
}

void OutputLog::writeSuccess(const QString& str)
{
	return write(str, OutputMessageLevel::Success, QString(""), 0, QString(""));
}

void OutputLog::writeWarning(const QString& str)
{
	return write(str, OutputMessageLevel::Warning, QString(""), 0, QString(""));
}

void OutputLog::writeError(const QString& str)
{
	return write(str, OutputMessageLevel::Error, QString(""), 0, QString(""));
}

void OutputLog::writeMessage(const QString& str, QString file, int fileLine, QString func)
{
	return write(str, OutputMessageLevel::Message, file, fileLine, func);
}

void OutputLog::writeSuccess(const QString& str, QString file, int fileLine, QString func)
{
	return write(str, OutputMessageLevel::Success, file, fileLine, func);
}

void OutputLog::writeWarning(const QString& str, QString file, int fileLine, QString func)
{
	return write(str, OutputMessageLevel::Warning, file, fileLine, func);
}

void OutputLog::writeError(const QString& str, QString file, int fileLine, QString func)
{
	return write(str, OutputMessageLevel::Error, file, fileLine, func);
}

void OutputLog::writeDump(const std::vector<quint8>& data)
{
	QString dataString;
	
	for (unsigned int i = 0 ; i < data.size(); i++)
	{
		if (i % 32 == 0 && i != 0)
		{
			writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
			dataString.clear();
		}

		dataString += (i %16 ? " " : " ' ")  + QString().setNum(data[i], 16).rightJustified(2, '0');

		if (i == data.size() - 1 && i % 32 > 0)	// last iteration
		{
			writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
			dataString.clear();
		}
	}
}

void OutputLog::writeEmptyLine()
{
	return writeMessage("");
}

bool OutputLog::isEmpty() const
{
	QMutexLocker locker(&m_mutex);
	return m_messages.empty();
}

OutputLogItem OutputLog::popMessages()
{
	QMutexLocker locker(&m_mutex);

	if (m_messages.empty() == true)
	{
		assert(m_messages.empty() == false);
		return OutputLogItem(0);
	}

	OutputLogItem logItem(m_messages.front());
	m_messages.pop_front();

	return logItem;
}

void OutputLog::startStrLogging()
{
	QMutexLocker locker(&m_mutex);

	m_strLogging = true;

	m_strFullLog.clear();
	m_strFullLog.reserve(10000);


}

QString OutputLog::finishStrLogging()
{
	QMutexLocker locker(&m_mutex);

	m_strLogging = false;

	QString str;
	str.swap(m_strFullLog);

	return str;
}

int OutputLog::errorCount() const
{
	QMutexLocker locker(&m_mutex);
	return m_errorCount;
}

void OutputLog::setErrorCount(int value)
{
	QMutexLocker locker(&m_mutex);
	int oldValue = m_errorCount;
	m_errorCount = value;
	locker.unlock();

	emit errorCountChanged(oldValue, value);

	return;
}

void OutputLog::incErrorCount()
{
	QMutexLocker locker(&m_mutex);
	int oldValue = m_errorCount;
	m_errorCount ++;
	locker.unlock();

	emit errorCountChanged(oldValue, m_errorCount);

	return;
}

void OutputLog::resetErrorCount()
{
	return setErrorCount(0);
}


int OutputLog::warningCount() const
{
	QMutexLocker locker(&m_mutex);
	return m_warningCount;
}

void OutputLog::setWarningCount(int value)
{
	QMutexLocker locker(&m_mutex);
	int oldValue = m_warningCount;
	m_warningCount = value;
	locker.unlock();

	emit warningCountChanged(oldValue, value);

	return;
}

void OutputLog::incWarningCount()
{
	QMutexLocker locker(&m_mutex);
	int oldValue = m_warningCount;
	m_warningCount ++;
	locker.unlock();

	emit warningCountChanged(oldValue, m_warningCount);

	return;
}

void OutputLog::resetWarningCount()
{
	return setWarningCount(0);
}


