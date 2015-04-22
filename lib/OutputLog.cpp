#include "../include/OutputLog.h"


OutputLogItem::OutputLogItem() :
	level(OutputMessageLevel::Message),
	bold(false)
{		
}

OutputLogItem::OutputLogItem(const QString& m, OutputMessageLevel l, bool b, const QDateTime& t) :
	message(m),
	level(l),
	bold(b),
	time(t)
{
}


OutputLog::OutputLog(void)
{
}


OutputLog::~OutputLog(void)
{
}

void OutputLog::clear()
{
	QMutexLocker locker(&mutex);

	windowMessageList.clear();
	fileMessageList.clear();

	return;
}

void OutputLog::write(const QString& str, OutputMessageLevel level, bool bold)
{
	QDateTime time = QDateTime::currentDateTime();

	// Get string level
	//
	QString strLevel;

	switch (level)
	{
	case OutputMessageLevel::Message:
		strLevel = "MSG";
		break;
	case OutputMessageLevel::Success:
		strLevel = "SCS";
		break;
	case OutputMessageLevel::Warning:
		strLevel = "WRN";
		break;
	case OutputMessageLevel::Error:
		strLevel = "ERR";
		break;
	default:
		assert(false);
	}

	// Set full string
	//
	QString message = QString("\n%1 %2: %3").arg(time.toString()).arg(strLevel).arg(str);

	qDebug() << message;

	QMutexLocker locker(&mutex);

	windowMessageList.push_back(OutputLogItem(str, level, bold, time));

	if (m_strLogging == true)
	{
		 m_strFullLog.append(message);
	}
	
	return;
}

void OutputLog::writeMessage(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Message, bold);
}

void OutputLog::writeEmptyLine()
{
	return write("", OutputMessageLevel::Message, false);
}

void OutputLog::writeSuccess(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Success, bold);
}

void OutputLog::writeWarning(const QString& str, bool bold, bool incWrnCounter)
{
	if (incWrnCounter == true)
	{
		setWarningCount(warningCount() + 1);
	}

	return write(str, OutputMessageLevel::Warning, bold);
}

void OutputLog::writeError(const QString& str, bool bold, bool incErrCounter)
{
	if (incErrCounter)
	{
		setErrorCount(errorCount() + 1);
	}

	return write(str, OutputMessageLevel::Error, bold);
}

void OutputLog::writeDump(const std::vector<quint8>& data)
{
	QString dataString;
	
	for (unsigned int i = 0 ; i < data.size(); i++)
	{
		if (i % 32 == 0 && i != 0)
		{
			writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString, false);
			dataString.clear();
		}

		dataString += (i %16 ? " " : " ' ")  + QString().setNum(data[i], 16).rightJustified(2, '0');

		if (i == data.size() - 1 && i % 32 > 0)	// last iteration
		{
			writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString, false);
			dataString.clear();
		}
	}
}

bool OutputLog::windowMessageListEmpty() const
{
	QMutexLocker locker(&mutex);
	return windowMessageList.empty();
}

OutputLogItem OutputLog::popWindowMessages()
{
	QMutexLocker locker(&mutex);

	if (windowMessageList.empty() == true)
	{
		assert(windowMessageList.empty() == false);
		return OutputLogItem();
	}

	OutputLogItem logItem(windowMessageList.front());
	windowMessageList.pop_front();

	return logItem;
}

void OutputLog::startStrLogging()
{
	QMutexLocker locker(&mutex);
	m_strLogging = true;

	m_strFullLog.clear();

	return;
}

QString OutputLog::finishStrLogging()
{
	QMutexLocker locker(&mutex);
	m_strLogging = false;

	QString tmp;
	std::swap(tmp, m_strFullLog);

	return tmp;
}

int OutputLog::errorCount() const
{
	QMutexLocker locker(&mutex);
	return m_errorCount;
}

void OutputLog::setErrorCount(int value)
{
	QMutexLocker locker(&mutex);
	int oldValue = m_errorCount;
	m_errorCount = value;
	locker.unlock();

	emit errorCountChanged(oldValue, value);

	return;
}

void OutputLog::resetErrorCount()
{
	return setErrorCount(0);
}


int OutputLog::warningCount() const
{
	QMutexLocker locker(&mutex);
	return m_warningCount;
}

void OutputLog::setWarningCount(int value)
{
	QMutexLocker locker(&mutex);
	int oldValue = m_warningCount;
	m_warningCount = value;
	locker.unlock();

	emit warningCountChanged(oldValue, value);

	return;
}

void OutputLog::resetWarningCount()
{
	return setWarningCount(0);
}


