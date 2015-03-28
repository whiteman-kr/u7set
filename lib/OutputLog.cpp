//#include "Stable.h"
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
#ifdef _DEBUG
	QString strLevel;
	switch (level)
	{
	case OutputMessageLevel::Message:
		strLevel = "Message";
		break;
	case OutputMessageLevel::Success:
		strLevel = "Success";
		break;
	case OutputMessageLevel::Warning:
		strLevel = "Warning";
		break;
	case OutputMessageLevel::Error:
		strLevel = "Error";
		break;
	default:
		assert(false);
	}
	qDebug() << QString("%1: %2").arg(strLevel).arg(str);
#endif

	QMutexLocker locker(&mutex);

	QDateTime time = QDateTime::currentDateTime();

	windowMessageList.push_back(OutputLogItem(str, level, bold, time));
	
	return;
}

void OutputLog::writeMessage(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Message, bold);
}

void OutputLog::writeSuccess(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Success, bold);
}

void OutputLog::writeWarning(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Warning, bold);
}

void OutputLog::writeError(const QString& str, bool bold)
{
	return write(str, OutputMessageLevel::Error, bold);
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
