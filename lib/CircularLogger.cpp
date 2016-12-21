#include "../lib/CircularLogger.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <assert.h>


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
	m_fileSizeLimit(fileSizeInMB)
{
}

CircularLoggerWorker::~CircularLoggerWorker()
{
	if (m_file != nullptr)
	{
		close();
		delete m_stream;
		delete m_file;
	}
}

void CircularLoggerWorker::writeRecord(const QString record)
{
	if (m_file == nullptr)
	{
		detectFiles();
	}
	*m_stream << record << '\n';

	//qDebug() << record;
}

void CircularLoggerWorker::close()
{
	if (m_file != nullptr)
	{
		writeLastRecord();
		m_file->close();
	}
}

void CircularLoggerWorker::flushStream()
{
	m_stream->flush();
}

void CircularLoggerWorker::detectFiles()
{
	QDir dir;
	if (!dir.exists(m_path))
	{
		dir.mkpath(m_path);
	}

	for (int i = 0; i < 1000; i++)
	{
		if (!QFile::exists(fileName(i)))
		{
			continue;
		}
		int id = getFileID(i);
		if (m_beginFileID == -1 || id < m_beginFileID)
		{
			m_beginFileID = id;
			m_beginFileNumber = i;
		}
		if (m_endFileID == -1 || id > m_endFileID)
		{
			m_endFileID = id;
			m_endFileNumber = i;
		}
	}

	if (m_beginFileID == -1)
	{
		m_beginFileID = 0;
		m_beginFileNumber = 0;
	}
	if (m_endFileID == -1)
	{
		m_endFileID = 0;
		m_endFileNumber = 0;
	}

	removeOldFiles();
	openFile(m_endFileNumber);
}

void CircularLoggerWorker::removeOldFiles()
{
	while (m_endFileID - m_beginFileID >= m_fileCount)
	{
		QFile::remove(fileName(m_beginFileNumber));
		m_beginFileNumber++;
		m_beginFileID++;
		if (m_beginFileNumber >= 1000)
		{
			m_beginFileNumber = 0;
		}
	}
}

void CircularLoggerWorker::checkFileSize()
{
	if (m_file->size() >= m_fileSizeLimit * 1024 * 1024)
	{
		close();
		m_endFileNumber++;
		m_endFileID++;
		if (m_endFileNumber >= 1000)
		{
			m_endFileNumber = 0;
		}
		QString newFileName = fileName(m_endFileNumber);
		if (QFile::exists(newFileName))
		{
			QFile::remove(newFileName);
		}
		removeOldFiles();
		openFile(m_endFileNumber);
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
	if (m_file == nullptr)
	{
		m_file = new QFile(fileName(index));
		m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
		m_stream = new QTextStream(m_file);

		QTimer* timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, &CircularLoggerWorker::flushStream);
		timer->start(1000);
	}
	else
	{
		m_file->setFileName(fileName(index));
		m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
	}
	if (m_file->size() == 0)
	{
		*m_stream << m_endFileID << '\n';
	}
	writeFirstRecord();
}

void CircularLoggerWorker::writeFirstRecord()
{
	*m_stream << tr("\nLOG OPN  ")
			  << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")
			  << "  \"by application "
			  << qApp->applicationFilePath()
			  << "\"\n";
}

void CircularLoggerWorker::writeLastRecord()
{
	*m_stream << tr("LOG CLS  ")
			  << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")
			  << "  \"by application "
			  << qApp->applicationFilePath()
			  << "\"\n";
	m_stream->flush();
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


void CircularLogger::init(QString logName, int fileCount, int fileSizeInMB, QString placementPath)
{
	if (m_loggerInitialized == true)
	{
		assert(false);			// Logger object is allready initialized.
		return;
	}

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


void CircularLogger::init(int fileCount, int fileSizeInMB, QString placementPath)
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

	init(fi.baseName(), fileCount, fileSizeInMB, placementPath);
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

	QString record = QString("%1 %2 \"%3\" \"FUNC=%4\" \"POS=%5:%6\"").
							arg(getCurrentDateTimeStr()).
							arg(getRecordTypeStr(type)).
							arg(message).
							arg(function).
							arg(file).
							arg(line);

	emit writeRecord(record);
}

