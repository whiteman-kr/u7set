#include "stable.h"
#include "Log.h"

Log theLog;


LogItem::LogItem() : 
	level(Message),
	bold(false)
{		
}

LogItem::LogItem(const QString& m, LogMessageLevel l, bool b, const QDateTime& t) :
	message(m),
	level(l),
	bold(b),
	time(t)
{
}


Log::Log(void)
{
}


Log::~Log(void)
{
}

void Log::write(const QString& str, LogMessageLevel level, bool bold)
{
	QMutexLocker locker(&mutex);

	QDateTime time = QDateTime::currentDateTime();

	windowMessageList.push_back(LogItem(str, level, bold, time));
	//fileMessageList.push_back(LogItem(str, level, time));
	
	return;
}

void Log::writeMessage(const QString& str, bool bold)
{
	return write(str, Message, bold);
}

void Log::writeSuccess(const QString& str, bool bold)
{
	return write(str, Success, bold);
}

void Log::writeWarning(const QString& str, bool bold)
{
	return write(str, Warning, bold);
}

void Log::writeError(const QString& str, bool bold)
{
	return write(str, Error, bold);
}

void Log::writeDump(const std::vector<uint8_t>& data)
{
	QString dataString;
	
	for (unsigned int i = 0 ; i < data.size(); i++)
	{
		if (i % 32 == 0 && i != 0)
		{
			theLog.writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
			dataString.clear();
		}

		dataString += (i %16 ? " " : " ' ")  + QString().setNum(data[i], 16).rightJustified(2, '0');

		if (i == data.size() - 1 && i % 32 > 0)	// last iteration
		{
			theLog.writeMessage(QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString);
			dataString.clear();							
		}
	}
}

bool Log::windowMessageListEmpty() const
{
	QMutexLocker locker(&mutex);
	return windowMessageList.empty();
}

LogItem Log::popWindowMessages()
{
	QMutexLocker locker(&mutex);

	if (windowMessageList.empty() == true)
	{
		assert(windowMessageList.empty() == false);
		return LogItem();
	}

	LogItem logItem(windowMessageList.front());
	windowMessageList.pop_front();

	return logItem;
}

