#pragma once

#include <QObject>
#include <QQueue>
#include <QMap>

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

	void close();

public slots:
	void writeRecord(const QString record);
	void flushStream();

private:
	QTextStream* m_stream = nullptr;
	QFile* m_file = nullptr;

	void detectFiles();
	void removeOldFiles();
	void checkFileSize();
	int getFileID(int index);
	QString fileName(int index);
	void openFile(int index);
	void writeFirstRecord();
	void writeLastRecord();

	QString m_logName;
	QString m_path;
	QString m_logFileName;
	int m_beginFileNumber = -1;
	int m_endFileNumber = -1;
	int m_fileCount;
	int m_fileSizeLimit;	// in megabytes
	qint64 m_beginFileID = -1;
	qint64 m_endFileID = -1;
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
};


#define LOG_ERR(str) logger.writeError(str, Q_FUNC_INFO, __FILE__, __LINE__);
#define LOG_WRN(str) logger.writeWarning(str, Q_FUNC_INFO, __FILE__, __LINE__);
#define LOG_MSG(str) logger.writeMessage(str, Q_FUNC_INFO, __FILE__, __LINE__);

#define INIT_LOGGER(appPath)	 logger.init(10, 10, appPath);


extern CircularLogger logger;
