#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <cassert>

#include "../lib/WUtils.h"
#include "../lib/CircularLogger.h"


// ----------------------------------------------------------------------------------
//
// CircularLoggerWorker class implementation
//
// ----------------------------------------------------------------------------------

CircularLoggerWorker::CircularLoggerWorker(QString logPath, QString logName, int fileCount, int fileSizeInMB) :
	m_logName(logName),
	m_path(logPath),
	m_fileCount(fileCount),
	m_fileSizeLimit(fileSizeInMB)
{
	if (m_fileCount < 1)
	{
		m_fileCount = 1;
	}

	if (m_fileCount > MAX_LOG_FILE_COUNT)
	{
		m_fileCount = MAX_LOG_FILE_COUNT;
	}

	if (m_fileSizeLimit < 1)
	{
		m_fileSizeLimit = 1;
	}

	if (m_fileSizeLimit > MAX_LOG_FILE_SIZE)
	{
		m_fileSizeLimit = MAX_LOG_FILE_SIZE;
	}

	assert(m_path.isEmpty() == false);
	assert(m_logName.isEmpty() == false);
}


CircularLoggerWorker::~CircularLoggerWorker()
{
	clearFileStream();
}


bool CircularLoggerWorker::writeFileCheck(const QString& logPath, const QString& logName)
{
	qint64 now = QDateTime::currentMSecsSinceEpoch();

	QString testFileName = logPath + '/' + logName + '_' + QString::number(now) + ".tmp";

	QFile testFile(testFileName);

	bool res = testFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

	if (res == false)
	{
		return false;
	}

	testFile.close();

	QDir dr;

	dr.remove(testFileName);

	return true;
}


void CircularLoggerWorker::writeRecord(const QString record)
{
	m_stream << record << '\n';

	m_stream.flush();

	m_fileGrowing += record.length();

	if (m_fileGrowing >= 10 * 1024)		// check each written 10k
	{
		checkFileSize();

		m_fileGrowing = 0;
	}
}


void CircularLoggerWorker::onThreadStarted()
{
	detectFiles();
}


void CircularLoggerWorker::onThreadFinished()
{
	clearFileStream();
}


void CircularLoggerWorker::detectFiles()
{
	QDir dir;

	if (dir.exists(m_path) == false)
	{
		dir.mkpath(m_path);
	}

	for (int i = 0; i < 1000; i++)
	{
		QString fName = fileName(i);

		if (QFile::exists(fName) == false)
		{
			continue;
		}

		int id = getFileID(i);

		if (m_firstFileID == -1 || id < m_firstFileID)
		{
			m_firstFileID = id;
			m_firstFileNumber = i;
		}

		if (m_lastFileID == -1 || id > m_lastFileID)
		{
			m_lastFileID = id;
			m_lastFileNumber = i;
		}
	}

	if (m_firstFileID == -1)
	{
		m_firstFileID = 0;
		m_firstFileNumber = 0;
	}
	if (m_lastFileID == -1)
	{
		m_lastFileID = 0;
		m_lastFileNumber = 0;
	}

	removeOldFiles();
	openFile(m_lastFileNumber);
}


void CircularLoggerWorker::removeOldFiles()
{
	while (m_lastFileID - m_firstFileID >= m_fileCount)
	{
		QFile::remove(fileName(m_firstFileNumber));

		m_firstFileNumber++;
		m_firstFileID++;

		if (m_firstFileNumber >= 1000)
		{
			m_firstFileNumber = 0;
		}
	}
}


void CircularLoggerWorker::checkFileSize()
{
	int fileSize = m_file.size();

	if (fileSize >= m_fileSizeLimit * 1024 * 1024)
	{
		clearFileStream();

		m_lastFileNumber++;
		m_lastFileID++;

		if (m_lastFileNumber >= 1000)
		{
			m_lastFileNumber = 0;
		}

		QString newFileName = fileName(m_lastFileNumber);

		if (QFile::exists(newFileName))
		{
			QFile::remove(newFileName);
		}

		removeOldFiles();

		openFile(m_lastFileNumber);
	}
}


int CircularLoggerWorker::getFileID(int index)
{
	QFile file(fileName(index));

	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream in(&file);

	return in.readLine().toInt();
}


QString CircularLoggerWorker::fileName(int index)
{
	return m_path + '/' + m_logName + '_' + QString("%1").arg(index, 3, 10, QChar('0')) + ".log";
}


void CircularLoggerWorker::openFile(int index)
{
	clearFileStream();

	m_fileName = fileName(index);

	m_file.setFileName(m_fileName);

	bool res = m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

	if (res == false)
	{
		qDebug() << "Error open file: " << C_STR(m_fileName);
	}

	m_stream.setDevice(&m_file);

	if (m_file.size() == 0)
	{
		writeRecord(QString("%1\n").arg(m_lastFileID));
	}
	else
	{
		writeRecord(QString(""));
	}
}


void CircularLoggerWorker::clearFileStream()
{
	m_stream.flush();
	m_file.close();
}


// ----------------------------------------------------------------------------------
//
// CircularLogger class implementation
//
// ----------------------------------------------------------------------------------

CircularLogger::CircularLogger()
{
}


CircularLogger::~CircularLogger()
{
}


bool CircularLogger::init(QString logName, int fileCount, int fileSizeInMB)
{
	if (m_loggerInitialized == true)
	{
		assert(false);				// Logger object is already initialized.
		return false;
	}

	if (qApp == nullptr)
	{
		assert(false);				// create QCoreApplication or QApplication instance first!
		return false;
	}

	QString appFileName = qApp->applicationFilePath();

	QFileInfo fi(appFileName);

	QString logPath = fi.absolutePath();

	if (logName.isEmpty() == true)
	{
		logName = fi.baseName();	// name log as app
	}

//	fi.setFile(logPath);

	if (CircularLoggerWorker::writeFileCheck(logPath, logName) == false)
	{
		logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

		qDebug() << "Application folder isn't writeble. Log created at: " << C_STR(logPath);
	}

	CircularLoggerWorker* worker = new CircularLoggerWorker(logPath, logName, fileCount, fileSizeInMB);

	addWorker(worker);

	connect(this, &CircularLogger::writeRecord, worker, &CircularLoggerWorker::writeRecord);

	start();

	m_loggerInitialized = true;

	return true;
}


bool CircularLogger::isInitialized() const
{
	return m_loggerInitialized;
}


void CircularLogger::shutdown()
{
	quitAndWait(500);
}


void CircularLogger::setLogCodeInfo(bool logCodeInfo)
{
	m_logCodeInfo = logCodeInfo;
}


void CircularLogger::writeError(const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	composeAndWriteRecord(RecordType::Error, message, function, file, line, debugEcho);
}


void CircularLogger::writeWarning(const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	composeAndWriteRecord(RecordType::Warning, message, function, file, line, debugEcho);
}


void CircularLogger::writeMessage(const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	composeAndWriteRecord(RecordType::Message, message, function, file, line, debugEcho);
}


QString CircularLogger::getRecordTypeStr(RecordType type)
{
	QString str;

	switch(type)
	{
	case RecordType::Error:
		str = "ERR";
		break;

	case RecordType::Warning:
		str = "WRN";
		break;

	case RecordType::Message:
		str = "MSG";
		break;

	case RecordType::Config:
		str = "CFG";
		break;

	default:
		str = "???";
		assert(false);
	}

	return str;
}


QString CircularLogger::getCurrentDateTimeStr()
{
	return QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
}


void CircularLogger::composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	if (m_loggerInitialized == false)
	{
		assert(false);		// Logger object isn't initialized. Call CircularLogger::init at first.
		return;
	}

	QString msg = message.simplified();

	msg.replace("&nbsp;", " ");

	if (debugEcho == true)
	{
		qDebug() << C_STR(msg);
	}

	QString record;

	if (m_logCodeInfo == true)
	{
		record = QString("%1 %2  %3  ###%4###%5:%6###").
							arg(getCurrentDateTimeStr()).
							arg(getRecordTypeStr(type)).
							arg(msg).
							arg(function).
							arg(file).
							arg(line);
	}
	else
	{
		record = QString("%1 %2  %3").
							arg(getCurrentDateTimeStr()).
							arg(getRecordTypeStr(type)).
							arg(msg);
	}


	emit writeRecord(record);
}


bool circularLoggerInit(std::shared_ptr<CircularLogger> logger, const QString& logName, int fileCount, int fileSizeInMB)
{
	if (logger != nullptr)
	{
		return logger->init(logName, fileCount, fileSizeInMB);
	}
	else
	{
#ifdef CIRCULAR_LOGGER_PTR_ASSERTING
		assert(false);
#endif
	}

	return false;
}


void circularLoggerShutdown(std::shared_ptr<CircularLogger> logger)
{
	if (logger != nullptr)
	{
		logger->shutdown();
	}
	else
	{
#ifdef CIRCULAR_LOGGER_PTR_ASSERTING
		assert(false);
#endif
	}
}


void circularLoggerWriteError(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	if (logger != nullptr)
	{
		logger->writeError(message, function, file, line, debugEcho);
	}
	else
	{
#ifdef CIRCULAR_LOGGER_PTR_ASSERTING
		assert(false);
#endif
		if (debugEcho == true)
		{
			qDebug() << C_STR(message);
		}
	}
}


void circularLoggerWriteWarning(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	if (logger != nullptr)
	{
		logger->writeWarning(message, function, file, line, debugEcho);
	}
	else
	{
#ifdef CIRCULAR_LOGGER_PTR_ASSERTING
		assert(false);
#endif
		if (debugEcho == true)
		{
			qDebug() << C_STR(message);
		}
	}
}


void circularLoggerWriteMessage(std::shared_ptr<CircularLogger> logger, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	if (logger != nullptr)
	{
		logger->writeMessage(message, function, file, line, debugEcho);
	}
	else
	{
#ifdef CIRCULAR_LOGGER_PTR_ASSERTING
		assert(false);
#endif
		if (debugEcho == true)
		{
			qDebug() << C_STR(message);
		}
	}
}

