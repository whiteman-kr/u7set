#ifndef LOGFILE_H
#define LOGFILE_H

#include "SimpleThread.h"

#include <QTimer>
#include <QMutex>

namespace Log
{
	enum class MessageType
	{
		Error,
		Warning,
		Message,
		Text
	};

	class LogFileWorker : public SimpleThreadWorker
	{
	public:

		LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount);
		~LogFileWorker();

		bool write(MessageType type, const QString& text);

	protected:
		virtual void onThreadStarted();
		virtual void onThreadFinished();

	private:

		QString getCurrentFileName() const;

		bool readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount);
		bool writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount);

		bool flush();

		void switchToNextLogFile();

	private slots:

		void slot_onTimer();

	private:

		QMutex m_mutex;

		QTimer m_timer;

		std::vector<std::pair<MessageType, QString>> m_queue;

		QString m_fileName;
		QString m_path;
		int m_maxFileSize;
		int m_maxFilesCount;

		int m_currentFileNumber = 0;

		const int m_serviceStringLength = 80;

		const int m_maxFileNumber = 9999;
	};

	class LogFile
	{
	public:

		LogFile(const QString& fileName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 3);
		~LogFile();

		bool writeMessage(const QString& text);
		bool writeError(const QString& text);
		bool writeWarning(const QString& text);
		bool writeText(const QString& text);

		bool write(MessageType type, const QString& text);

	private:

		LogFileWorker* m_logFileWorker = nullptr;
		SimpleThread m_logThread;
	};

}

#endif // LOGFILE_H

