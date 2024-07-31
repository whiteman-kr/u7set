#include <WUtils.h>
#include "CircularLogger.h"

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

bool CircularLogger::init(const QString& appPathFile, const QString& logName, const QString& instanceID,
						  int fileCount, int fileSizeInMB)
{
	if (m_loggerInitialized == true)
	{
		assert(false);				// Logger object is already initialized.
		return false;
	}

	QFileInfo fi(appPathFile);

	QString logPath = fi.absolutePath();

	m_logName = logName;

	if (m_logName.isEmpty() == true)
	{
		m_logName = fi.baseName();	// name log as app
	}

	if (instanceID.isEmpty() == false)
	{
		m_logName.append(QString("_%1").arg(instanceID));
	}

	if (writeFileCheck(logPath, logName) == false)
	{
		logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

		qDebug() << "Application folder isn't writeble. Log created at: " << C_STR(logPath);
	}

	m_path = logPath;
	m_fileCount = fileCount;
	m_fileSizeLimit = fileSizeInMB;

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

	m_loggerInitialized = true;

	m_logWriteThread = std::thread(&CircularLogger::logWriteThreadFunc, this);

	return true;
}

bool CircularLogger::isInitialized() const
{
	return m_loggerInitialized;
}

void CircularLogger::shutdown()
{
	{
		std::lock_guard lg(m_waitConditionMutex);
		m_quitRequested = true;
		m_waitCondition.notify_one();
	}

	m_logWriteThread.join();
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

void CircularLogger::writeRecord(QString record)
{
	record += "\n";

	m_file.write(record.toUtf8());
	m_file.flush();

	m_fileGrowing += record.length();

	if (m_fileGrowing >= 10 * 1024)		// check each written 10k
	{
		checkFileSize();

		m_fileGrowing = 0;
	}
}

bool CircularLogger::writeFileCheck(const QString& logPath, const QString& logName)
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

void CircularLogger::detectFiles()
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

void CircularLogger::removeOldFiles()
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

void CircularLogger::checkFileSize()
{
	qint64 fileSize = m_file.size();

	if (fileSize >= m_fileSizeLimit * 1024 * 1024)
	{
		closeFile();

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

int CircularLogger::getFileID(int index)
{
	QFile file(fileName(index));

	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream in(&file);

	return in.readLine().toInt();
}

QString CircularLogger::fileName(int index)
{
	return m_path + '/' + m_logName + '_' + QString("%1").arg(index, 3, 10, QChar('0')) + ".log";
}

void CircularLogger::openFile(int index)
{
	if (m_file.isOpen() == true)
	{
		closeFile();
	}

	m_fileName = fileName(index);

	m_file.setFileName(m_fileName);

	bool res = m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

	if (res == false)
	{
		qDebug() << "Error open file: " << C_STR(m_fileName);
	}

	if (m_file.size() == 0)
	{
		writeRecord(QString("%1\n").arg(m_lastFileID));
	}
	else
	{
		writeRecord(QString(""));
	}
}

void CircularLogger::closeFile()
{
	m_file.flush();
	m_file.close();
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
	QDateTime&& currentTime = QDateTime::currentDateTime();
	if (currentTime.isValid() == false)
	{
		return "Current time is not valid";
	}
	assert(currentTime.isValid());
	QDate&& date = currentTime.date();
	QTime&& time = currentTime.time();
	QChar&& zero = QChar('0');
	return QString("%1.%2.%3 %4:%5:%6.%7")
			.arg(date.year(), 4, 10, zero)
			.arg(date.month(), 2, 10, zero)
			.arg(date.day(), 2, 10, zero)
			.arg(time.hour(), 2, 10, zero)
			.arg(time.minute(), 2, 10, zero)
			.arg(time.second(), 2, 10, zero)
			.arg(time.msec(), 3, 10, zero);
	//return QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
}

void CircularLogger::composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line, bool debugEcho)
{
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

	std::lock_guard lg(m_waitConditionMutex);

	m_recordsQueue.push(record);

	m_waitCondition.notify_one();
}

void CircularLogger::logWriteThreadFunc()
{
	detectFiles();

	std::unique_lock ul(m_waitConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		m_waitCondition.wait(ul, [this]() -> bool
						   {
							   return m_quitRequested ||
									  m_recordsQueue.empty() == false;
						   });

		if (m_quitRequested == true)
		{
			while(m_recordsQueue.empty() == false)
			{
				writeRecord(m_recordsQueue.front());
				m_recordsQueue.pop();
			}

			ul.unlock();
			break;
		}

		if (m_recordsQueue.empty() == true)
		{
			continue;
		}

		QString record = m_recordsQueue.front();
		m_recordsQueue.pop();

		ul.unlock();

		writeRecord(record);
	}

	closeFile();
}

bool circularLoggerInit(std::shared_ptr<CircularLogger> logger,
						const QString& appPathFile,
						const QString& logName,
						const QString& instanceID,
						int fileCount,
						int fileSizeInMB)
{
	if (logger != nullptr)
	{
		return logger->init(appPathFile, logName, instanceID, fileCount, fileSizeInMB);
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



