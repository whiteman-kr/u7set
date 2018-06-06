#pragma once

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QSystemSemaphore>
#include <memory>

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
	CircularLoggerWorker(QString logPath, QString logName, int fileCount, int fileSizeInMB);
	~CircularLoggerWorker();

	static bool writeFileCheck(const QString& logPath, const QString& logName);


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
	int m_fileGrowing = 0;

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

	bool init(QString logName, int fileCount, int fileSizeInMB);

	bool isInitialized() const;

	void shutdown();

	void setLogCodeInfo(bool logCodeInfo);

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
	bool m_logCodeInfo = true;
};

typedef std::shared_ptr<CircularLogger> CircularLoggerShared;


// To asserting "logger != nullptr" inside circularLogger* functions define symbol CIRCULAR_LOGGER_PTR_ASSERTING in your project:
//
// #define CIRCULAR_LOGGER_PTR_ASSERTING
//
bool circularLoggerInit(std::shared_ptr<CircularLogger> logger, const QString& logName, int fileCount, int fileSizeInMB);
void circularLoggerShutdown(std::shared_ptr<CircularLogger> logger);

void circularLoggerWriteError(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho);
void circularLoggerWriteWarning(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho);
void circularLoggerWriteMessage(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho);


#define LOG_ERR(logger, str)		circularLoggerWriteError(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_WRN(logger, str)		circularLoggerWriteWarning(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_MSG(logger, str)		circularLoggerWriteMessage(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, false);
#define LOG_CALL(logger)			circularLoggerWriteMessage(logger, Q_FUNC_INFO, Q_FUNC_INFO, __FILE__, __LINE__, false);

#define DEBUG_LOG_ERR(logger, str)	circularLoggerWriteError(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_WRN(logger, str)	circularLoggerWriteWarning(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_MSG(logger, str)	circularLoggerWriteMessage(logger, str, Q_FUNC_INFO, __FILE__, __LINE__, true);
#define DEBUG_LOG_CALL(logger)		circularLoggerWriteMessage(logger, Q_FUNC_INFO, Q_FUNC_INFO, __FILE__, __LINE__, true);

#define LOGGER_INIT(logger)				circularLoggerInit(logger, QString(), 10, 10);
#define LOGGER_INIT2(logger, logName)	circularLoggerInit(logger, logName, 10, 10);

#define LOGGER_SHUTDOWN(logger)		circularLoggerShutdown(logger);
