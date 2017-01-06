#pragma once

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QSystemSemaphore>

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

private:
	QString m_fileName;
	QFile m_file;
	QTextStream m_stream;

	QString m_logName;
	QString m_path;

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

	void init(QString logName, int fileCount, int fileSizeInMB, QString placementPath = "");
	void init(int fileCount, int fileSizeInMB, QString placementPath = "");

	void shutdown();

	bool isInitialized() const { return m_loggerInitialized; }

signals:
	void writeRecord(const QString record);

public:
	void writeError(const QString& message, const char* function, const char* file, int line, bool debugEcho);
	void writeWarning(const QString& message, const char* function, const char* file, int line, bool debugEcho);
	void writeMessage(const QString& message, const char* function, const char* file, int line, bool debugEcho);

private:
	QString getRecordTypeStr(RecordType type);
	QString getCurrentDateTimeStr();

	void composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line, bool debugEcho);

private:
	bool m_loggerInitialized = false;
};


#define LOG_ERR(str) logger.writeError(str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_WRN(str) logger.writeWarning(str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_MSG(str) logger.writeMessage(str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_CALL() logger.writeMessage(Q_FUNC_INFO, Q_FUNC_INFO, __FILE__, __LINE__, false);

#define DEBUG_LOG_ERR(str) logger.writeError(str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_WRN(str) logger.writeWarning(str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_MSG(str) logger.writeMessage(str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_CALL() logger.writeMessage(Q_FUNC_INFO, Q_FUNC_INFO, __FILE__, __LINE__, true);

#define INIT_LOGGER(appPath)		logger.init(10, 10, appPath);
#define SHUTDOWN_LOGGER				logger.shutdown();


extern CircularLogger logger;
