#pragma once

#include <QObject>
#include <QTimer>

#include "SimpleThread.h"

class QFile;
class QTextStream;

// ----------------------------------------------------------------------------------
//
// CircularLoggerWorker class declaration
//
// ----------------------------------------------------------------------------------

class CircularLoggerWorker : public SimpleThreadWorker
{
	Q_OBJECT
public:
	CircularLoggerWorker(QString logName, int fileCount, int fileSizeInMB, QString placementPath = "");
	~CircularLoggerWorker();

public slots:
	void writeRecord(const QString record);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	void detectFiles();
	void removeOldFiles();
	void checkFileSize();
	int getFileID(int index);
	QString fileName(int index);
	void openFile(int index);
	void clearFileStream();

private slots:
	void flushStream();

private:
	QTextStream* m_stream = nullptr;
	QFile* m_file = nullptr;
	QTimer m_timer;

	QString m_logName;
	QString m_path;
	QString m_logFileName;

	const int MAX_LOG_FILE_COUNT = 10;
	const int MAX_LOG_FILE_SIZE = 10;		// in megabytes

	int m_fileCount = 0;
	int m_fileSizeLimit = 0;				// in megabytes

	int m_firstFileNumber = -1;
	int m_lastFileNumber = -1;
	qint64 m_firstFileID = -1;
	qint64 m_lastFileID = -1;
};


// ----------------------------------------------------------------------------------
//
// CircularLogger class declaration
//
// ----------------------------------------------------------------------------------

class CircularLogger : public SimpleThread
{
	Q_OBJECT

public:
	enum class RecordType
	{
		Error,
		Warning,
		Message,
		Config
	};

public:
	CircularLogger();
	~CircularLogger();

	void init(QString logName, int fileCount, int fileSizeInMB, QString placementPath = "", bool echoToDebug = false);
	void init(int fileCount, int fileSizeInMB, QString placementPath = "", bool echoToDebug = false);

	bool isInitialized() const { return m_loggerInitialized; }

signals:
	void writeRecord(const QString record);

public:
	void writeError(const QString& message, const char* function, const char* file, int line);
	void writeWarning(const QString& message, const char* function, const char* file, int line);
	void writeMessage(const QString& message, const char* function, const char* file, int line);
	void writeConfig(const QString& message, const char* function, const char* file, int line);

private:
	QString getRecordTypeStr(RecordType type);
	QString getCurrentDateTimeStr();

	void composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line);

private:
	bool m_loggerInitialized = false;
	bool m_echoToDebug = false;
};


#define LOG_ERR(str) logger.writeError(str, Q_FUNC_INFO, __FILE__, __LINE__);
#define LOG_WRN(str) logger.writeWarning(str, Q_FUNC_INFO, __FILE__, __LINE__);
#define LOG_MSG(str) logger.writeMessage(str, Q_FUNC_INFO, __FILE__, __LINE__);
#define LOG_CALL() logger.writeMessage(Q_FUNC_INFO, Q_FUNC_INFO, __FILE__, __LINE__);

#define INIT_LOGGER(appPath, echoToDebug)	 logger.init(10, 10, appPath, echoToDebug);


extern CircularLogger logger;
