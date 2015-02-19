#include "Stable.h"
#include "../include/OutputLog.h"


OutputLogItem::OutputLogItem() :
	level(Message),
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
	QMutexLocker locker(&mutex);

	QDateTime time = QDateTime::currentDateTime();

	windowMessageList.push_back(OutputLogItem(str, level, bold, time));
	//fileMessageList.push_back(OutputLogItem(str, level, time));
	
	return;
}

void OutputLog::writeMessage(const QString& str, bool bold)
{
	return write(str, Message, bold);
}

void OutputLog::writeSuccess(const QString& str, bool bold)
{
	return write(str, Success, bold);
}

void OutputLog::writeWarning(const QString& str, bool bold)
{
	return write(str, Warning, bold);
}

void OutputLog::writeError(const QString& str, bool bold)
{
	return write(str, Error, bold);
}

void OutputLog::writeDump(const std::vector<char>& /*data*/)
{
//	QString dataString;
	
//	for (unsigned int i = 0 ; i < data.size(); i++)
//	{
//		if (i % 32 == 0 && i != 0)
//		{
//			theLog.writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
//			dataString.clear();
//		}

//		dataString += (i %16 ? " " : " ' ")  + QString().setNum(data[i], 16).rightJustified(2, '0');

//		if (i == data.size() - 1 && i % 32 > 0)	// last iteration
//		{
//			theLog.writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
//			dataString.clear();
//		}
//	}
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
