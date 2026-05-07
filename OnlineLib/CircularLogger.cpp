#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "../UtilsLib/WUtils.h"
#include "CircularLogger.h"

// ----------------------------------------------------------------------------------
//
// CircularLoggerWorker class implementation
//
// ----------------------------------------------------------------------------------

CircularLoggerWorker::CircularLoggerWorker(const QString& logPath, const QString& logName, int fileCount, int fileSizeInMB) :
	SimpleThreadWorker("CircularLoggerWorker"),
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

void CircularLoggerWorker::writeRecord(const QString& record)
{
	if (m_stream.device() == nullptr)
	{
		return;
	}

	m_buffer.push_back(record);
	m_bufferChars += record.size();

	if (m_buffer.size() >= MAX_BUFFER_LINES || m_bufferChars >= MAX_BUFFER_CHARS)
	{
		flushBuffer();
	}
}

void CircularLoggerWorker::onThreadStarted()
{
	detectFiles();

	m_flushTimer = new QTimer(this);
	m_flushTimer->setTimerType(Qt::CoarseTimer);
	m_flushTimer->setInterval(FLUSH_INTERVAL_MS);

	connect(m_flushTimer, &QTimer::timeout, this, &CircularLoggerWorker::flushBuffer);

	m_flushTimer->start();
}

void CircularLoggerWorker::onThreadFinished()
{
	if (m_flushTimer != nullptr)
	{
		m_flushTimer->stop();
		m_flushTimer = nullptr;
	}

	clearFileStream();
}

void CircularLoggerWorker::detectFiles()
{
	QDir dir(m_path);

	if (dir.exists() == false)
	{
		if (dir.mkpath(".") == false)
		{
			qDebug() << "Can't create log folder:" << C_STR(m_path);
			return;
		}
	}

	for (int i = 0; i <= MAX_LOG_FILE_INDEX; i++)
	{
		QString fName = fileName(i);

		if (QFile::exists(fName) == false)
		{
			continue;
		}

		int id = getFileID(i);

		if (id == -1)
		{
			QFile::remove(fName);
			continue;
		}

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

		if (m_firstFileNumber > MAX_LOG_FILE_INDEX)
		{
			m_firstFileNumber = 0;
		}
	}
}

void CircularLoggerWorker::checkFileSize()
{
	qint64 fileSize = m_file.size();

	if (fileSize >= m_fileSizeLimit * 1024 * 1024)
	{
		clearFileStream();

		m_lastFileNumber++;
		m_lastFileID++;

		if (m_lastFileNumber > MAX_LOG_FILE_INDEX)
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

	if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		return -1;
	}

	QTextStream in(&file);

	if (in.atEnd())
	{
		return -1;
	}

	bool ok = false;

	const int id = in.readLine().toInt(&ok);

	if (ok == false)
	{
		return -1;
	}

	return id;
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
		Q_ASSERT(false);
		m_stream.setDevice(nullptr);
		return;
	}

	m_stream.setDevice(&m_file);

	if (m_file.size() == 0)
	{
		m_stream << QString("%1\n\n").arg(m_lastFileID);	// direct writing, no buffering!
	}
	else
	{
		m_stream << QString("\n");							// direct writing, no buffering!
	}

	m_stream.flush();
}

void CircularLoggerWorker::clearFileStream()
{
	flushBuffer();

	if (m_file.isOpen())
	{
		m_stream.flush();
		m_file.close();
	}

	m_stream.setDevice(nullptr);
}

void CircularLoggerWorker::flushBuffer()
{
	if (m_stream.device() == nullptr)
	{
		m_buffer.clear();
		m_bufferChars = 0;
		return;
	}

	if (m_buffer.isEmpty())
	{
		return;
	}

	for (const QString& s : m_buffer)
	{
		m_stream << s << '\n';
	}

	m_stream.flush();

	m_fileGrowing += m_bufferChars;

	m_buffer.clear();
	m_bufferChars = 0;

	if (m_fileGrowing >= FILE_GROWING_CHECK_SIZE)
	{
		checkFileSize();
		m_fileGrowing = 0;
	}
}

// ----------------------------------------------------------------------------------
//
// CircularLogger class implementation
//
// ----------------------------------------------------------------------------------

CircularLogger::CircularLogger()
{
}

CircularLogger::CircularLogger(ILogFile* externalLog, QString context) :
	m_externalLogger({externalLog, context})
{
	Q_ASSERT(externalLog);
}

CircularLogger::~CircularLogger()
{
}

bool CircularLogger::init(QString logName, QString instanceID, int fileCount, int fileSizeInMB)
{
	if (m_externalLogger.has_value() == true)
	{
		return false;
	}

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

	if (instanceID.isEmpty() == false)
	{
		logName.append(QString("_%1").arg(instanceID));
	}

	if (CircularLoggerWorker::writeFileCheck(logPath, logName) == false)
	{
		logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

		qDebug() << "Application folder isn't writeble. Log created at: " << C_STR(logPath);
	}

	CircularLoggerWorker* worker = new CircularLoggerWorker(logPath, logName, fileCount, fileSizeInMB);

	addWorker(worker);

	connect(this, &CircularLogger::writeRecord, worker, &CircularLoggerWorker::writeRecord, Qt::QueuedConnection);

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
	if (m_externalLogger.has_value() == true)
	{
		return;
	}

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
	const QDateTime currentTime = QDateTime::currentDateTime();

	const QDate date = currentTime.date();
	const QTime time = currentTime.time();

	const static QChar ZERO('0');

	return QString("%1.%2.%3 %4:%5:%6.%7").
			arg(date.year(), 4, 10, ZERO).
			arg(date.month(), 2, 10, ZERO).
			arg(date.day(), 2, 10, ZERO).
			arg(time.hour(), 2, 10, ZERO).
			arg(time.minute(), 2, 10, ZERO).
			arg(time.second(), 2, 10, ZERO).
			arg(time.msec(), 3, 10, ZERO);
}

void CircularLogger::composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
	if (m_externalLogger.has_value() == true)
	{
		switch (type)
		{
		case RecordType::Error:
			m_externalLogger->writeError(message);
			break;
		case RecordType::Warning:
			m_externalLogger->writeWarning(message);
			break;
		case RecordType::Message:
			m_externalLogger->writeMessage(message);
			break;
		case RecordType::Config:
			m_externalLogger->writeText(message);
			break;
		}

		return;
	}

	if (m_loggerInitialized == false)
	{
		assert(false);		// Logger object isn't initialized. Call CircularLogger::init at first.
		return;
	}

	QString msg = message;

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

bool circularLoggerInit(std::shared_ptr<CircularLogger> logger,
						const QString& logName,
						const QString& instanceID,
						int fileCount,
						int fileSizeInMB)
{
	if (logger != nullptr)
	{
		return logger->init(logName, instanceID, fileCount, fileSizeInMB);
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
			std::cout << C_STR(QString("%1\n").arg(message));
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
			std::cout << C_STR(QString("%1\n").arg(message));
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
			std::cout << C_STR(QString("%1\n").arg(message));
		}
	}
}

// ----------------------------------------------------------------------------------
//
// LogWrapper class implementation
//
// ----------------------------------------------------------------------------------

LogWrapper::LogWrapper(CircularLoggerShared log, const QString& className) :
	m_log(log),
	m_className(className)
{
}

LogWrapper::~LogWrapper()
{
}

CircularLoggerShared LogWrapper::getLog() const
{
	return m_log;
}

void LogWrapper::setLog(CircularLoggerShared log)
{
	m_log = log;
}

void LogWrapper::logMsg(const QString& msg) const
{
	if (m_log)
	{
		if (m_className.isEmpty())
		{
			DEBUG_LOG_MSG(m_log, msg);
		}
		else
		{
			DEBUG_LOG_MSG(m_log, QString("%1: %2").arg(m_className).arg(msg));
		}
	}
}

void LogWrapper::logWrn(const QString& wrn) const
{
	if (m_log)
	{
		if (m_className.isEmpty())
		{
			DEBUG_LOG_WRN(m_log, wrn);
		}
		else
		{
			DEBUG_LOG_WRN(m_log, QString("%1: %2").arg(m_className).arg(wrn));
		}
	}
}

void LogWrapper::logErr(const QString& err) const
{
	if (m_log)
	{
		if (m_className.isEmpty())
		{
			DEBUG_LOG_ERR(m_log, err);
		}
		else
		{
			DEBUG_LOG_ERR(m_log, QString("%1: %2").arg(m_className).arg(err));
		}
	}
}
