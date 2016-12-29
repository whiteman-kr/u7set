#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <assert.h>

#include "../lib/WUtils.h"
#include "../lib/CircularLogger.h"

CircularLogger logger;		// global logger object

// ----------------------------------------------------------------------------------
//
// CircularLoggerWorker class implementation
//
// ----------------------------------------------------------------------------------

CircularLoggerWorker::CircularLoggerWorker(QString logName, int fileCount, int fileSizeInMB, QString placementPath) :
	m_logName(logName),
	m_path(placementPath.isEmpty() ? qApp->applicationDirPath() + "/Log" : placementPath),
	m_fileCount(fileCount),
	m_fileSizeLimit(fileSizeInMB),
	m_timer(this)
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
}


CircularLoggerWorker::~CircularLoggerWorker()
{
	clearFileStream();
}


void CircularLoggerWorker::writeRecord(const QString record)
{
	if (m_stream != nullptr)
	{
		*m_stream << record << '\n';
	}
}


void CircularLoggerWorker::onThreadStarted()
{
	detectFiles();

	connect(&m_timer, &QTimer::timeout, this, &CircularLoggerWorker::flushStream);
	m_timer.start(1000);
}


void CircularLoggerWorker::onThreadFinished()
{
	clearFileStream();
}


void CircularLoggerWorker::flushStream()
{
	m_stream->flush();
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
		if (QFile::exists(fileName(i)) == false)
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
	if (m_file->size() >= m_fileSizeLimit * 1024 * 1024)
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

	m_file = new QFile(fileName(index));
	m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
	m_stream = new QTextStream(m_file);

	if (m_file->size() == 0)
	{
		*m_stream << m_lastFileID << '\n';
	}

	*m_stream << '\n';
}


void CircularLoggerWorker::clearFileStream()
{
	if (m_stream != nullptr)
	{
		m_stream->flush();
		delete m_stream;
		m_stream = nullptr;
	}

	if (m_file != nullptr)
	{
		m_file->close();
		delete m_file;
		m_file = nullptr;
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


CircularLogger::~CircularLogger()
{
	quitAndWait();
}


void CircularLogger::init(QString logName, int fileCount, int fileSizeInMB, QString placementPath, bool echoToDebug)
{
	if (m_loggerInitialized == true)
	{
		assert(false);			// Logger object is allready initialized.
		return;
	}

	m_echoToDebug = echoToDebug;

	if (placementPath.isEmpty())
	{
		placementPath = qApp->applicationDirPath();
	}

	assert(placementPath.isEmpty() == false);

	QFileInfo fi(placementPath);
	QString logPath;

	if (fi.isRelative())
	{
		logPath = qApp->applicationDirPath() + "/" + placementPath;
	}
	else
	{
		if (fi.isDir())
		{
			logPath = fi.absoluteFilePath();
		}
		else
		{
			logPath = fi.absolutePath();
		}
	}

	CircularLoggerWorker* worker = new CircularLoggerWorker(logName, fileCount, fileSizeInMB, logPath);

	addWorker(worker);

	connect(this, &CircularLogger::writeRecord,
			worker, &CircularLoggerWorker::writeRecord,
			Qt::QueuedConnection);

	start();

	m_loggerInitialized = true;
}


void CircularLogger::init(int fileCount, int fileSizeInMB, QString placementPath, bool echoToDebug)
{
	if (m_loggerInitialized == true)
	{
		assert(false);			// Logger object is allready initialized.
		return;
	}

	QString filePath;

	if (qApp != nullptr && !qApp->applicationFilePath().isEmpty())
	{
		filePath = qApp->applicationFilePath();
	}

	if (filePath.isEmpty() == true)
	{
		assert(!placementPath.isEmpty());
		filePath = placementPath;
	}

	QFileInfo fi(filePath);

	if (!fi.isFile())
	{
		assert(fi.isFile());
		return;
	}

	init(fi.baseName(), fileCount, fileSizeInMB, placementPath, echoToDebug);
}


void CircularLogger::writeError(const QString& message, const char* function, const char* file, int line)
{
	composeAndWriteRecord(RecordType::Error, message, function, file, line);
}


void CircularLogger::writeWarning(const QString& message, const char* function, const char* file, int line)
{
	composeAndWriteRecord(RecordType::Warning, message, function, file, line);
}


void CircularLogger::writeMessage(const QString& message, const char* function, const char* file, int line)
{
	composeAndWriteRecord(RecordType::Message, message, function, file, line);
}


void CircularLogger::writeConfig(const QString& message, const char* function, const char* file, int line)
{
	composeAndWriteRecord(RecordType::Config, message, function, file, line);
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


void CircularLogger::composeAndWriteRecord(RecordType type, const QString& message, const char* function, const char* file, int line)
{
	if (m_loggerInitialized == false)
	{
		assert(false);		// Logger object isn't initialized. Call CircularLogger::init at first.
		return;
	}

	if (m_echoToDebug == true)
	{
		qDebug() << C_STR(QString("%1").arg(message));
	}

	QString record = QString("\"%1\" \"%2\" \"%3\" \"%4\" \"%5:%6\"").
							arg(getCurrentDateTimeStr()).
							arg(getRecordTypeStr(type)).
							arg(message).
							arg(function).
							arg(file).
							arg(line);

	emit writeRecord(record);
}

