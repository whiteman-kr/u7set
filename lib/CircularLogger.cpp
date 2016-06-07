#include "../lib/CircularLogger.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <assert.h>


CircularLogger logger;


const char* const MessageTypeStr[] =
{
	"USR",
	"NET",
	"APP"
};

const int MESSAGE_TYPE_COUNT = sizeof(MessageTypeStr) / sizeof(MessageTypeStr[0]);


const char* CategoryTypeStr[] =
{
	"ERR",
	"WRN",
	"MSG",
	"CFG"
};

const int CATEGORY_TYPE_COUNT = sizeof(CategoryTypeStr) / sizeof(CategoryTypeStr[0]);


CircularLogger::CircularLogger(QObject *parent) :
	QObject(parent)
{

}

CircularLogger::~CircularLogger()
{
	if (m_thread != nullptr)
	{
		m_thread->quit();
		m_thread->wait();
	}

	if (m_circularLoggerWorker != nullptr)
	{
		delete m_circularLoggerWorker;
	}
}

void CircularLogger::initLog(QString logName, int fileCount, int fileSizeInMB, QString placementPath)
{
	if (placementPath.isEmpty())
	{
		placementPath = qApp->applicationDirPath();
	}
	assert(!placementPath.isEmpty());

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

	m_circularLoggerWorker = new CircularLoggerWorker(logName, fileCount, fileSizeInMB, logPath);
	m_thread = new QThread(this);
	m_thread->start();
	m_circularLoggerWorker->moveToThread(m_thread);

	connect(this, &CircularLogger::writeRecord,
			m_circularLoggerWorker, &CircularLoggerWorker::writeRecord,
			Qt::QueuedConnection);
}

void CircularLogger::initLog(int fileCount, int fileSizeInMB, QString placementPath)
{
	QString filePath;
	if (qApp != nullptr && !qApp->applicationFilePath().isEmpty())
	{
		filePath = qApp->applicationFilePath();
	}
	if (filePath.isEmpty())
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

	initLog(fi.baseName(), fileCount, fileSizeInMB, placementPath);
}

QString CircularLogger::composeRecord(int type, int category, const QString &function, const QString &message)
{
	QString record;

	if (type >= 0 && type < MESSAGE_TYPE_COUNT)
	{
		record = tr(MessageTypeStr[type]);
	}
	else
	{
		Q_ASSERT(false);
		record = "???";
	}

	record += ' ';

	if (category >= 0 && category < CATEGORY_TYPE_COUNT)
	{
		record += CategoryTypeStr[category];
	}
	else
	{
		Q_ASSERT(false);
		record += "???";
	}

	record += "  ";
	record += QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");

	record += "  \"";
	record += message;
	record += '\"';

	if (!function.isEmpty())
	{
		record += " \"FN=";
		record += function;
		record += '\"';
	}
	else
	{
		Q_ASSERT(false);
	}

	return record;
}


CircularLoggerWorker::CircularLoggerWorker(QString logName, int fileCount, int fileSizeInMB, QString placementPath) :
	QObject(nullptr),
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
