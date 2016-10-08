#include "LogFile.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

LogFile::LogFile(const QString& fileName, const QString& path, int maxFileSize)
	:m_fileName(fileName),
	  m_path(path),
	  m_maxFileSize(maxFileSize)
{


}

LogFile::~LogFile()
{
	QMutexLocker l(&m_mutex);
	flush();
}

enum MessageType
{
	Error,
	Warning,
	Message,
	Text
};

bool LogFile::writeMessage(const QString& text)
{
	return write(MessageType::Message, text);
}

bool LogFile::writeError(const QString& text)
{
	return write(MessageType::Error, text);
}

bool LogFile::writeWarning(const QString& text)
{
	return write(MessageType::Warning, text);
}

bool LogFile::write(const QString& text)
{
	return write(MessageType::Text, text);
}

bool LogFile::write(MessageType type, const QString& text)
{
	QMutexLocker l(&m_mutex);
	m_queue.push_back(std::make_pair(type, text));

	if (m_queue.size() >= 10)
	{
		return flush();
	}

	return true;
}

bool LogFile::flush()
{
	if (m_queue.size() == 0)
	{
		return true;
	}

	QString timeStr = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

	QFile file(m_path + QDir::separator() + m_fileName + ".log");

	if (file.size() > m_maxFileSize)
	{
		file.rename(m_fileName + timeStr + ".log");
	}

	if (file.open(QIODevice::Append | QIODevice::Text ) == false)
	{
		return false;
	}

	QTextStream logStream(&file);

	for (auto it : m_queue)
	{

		const MessageType& type = it.first;

		if (type == MessageType::Text)
		{
			logStream << it.second << "\r\n";
			continue;
		}

		logStream << timeStr << "\t";

		switch(type)
		{

		case MessageType::Error:
			logStream << "ERR\t";
			break;
		case MessageType::Warning:
			logStream << "WRN\t";
			break;
		case MessageType::Message:
			logStream << "MSG\t";
			break;
		}


		logStream << it.second << "\r\n";
	}

	file.close();

	m_queue.clear();

	return true;
}
