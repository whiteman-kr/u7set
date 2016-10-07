#ifndef LOGFILE_H
#define LOGFILE_H

#include <QMutex>

class LogFile
{
public:
	LogFile(const QString &fileName, const QString &path, int maxFileSize = 1048576);
	~LogFile();

	enum class MessageType
	{
		Error,
		Warning,
		Message,
		Text
	};

	bool writeMessage(const QString& text);
	bool writeError(const QString& text);
	bool writeWarning(const QString& text);


	bool write(MessageType type, const QString& text);
	bool write(const QString& text);

private:
	bool flush();

private:

	QMutex m_mutex;

	std::vector<std::pair<MessageType, QString>> m_queue;

	QString m_fileName;
	QString m_path;
	int m_maxFileSize = 1048576;
};

#endif // LOGFILE_H
