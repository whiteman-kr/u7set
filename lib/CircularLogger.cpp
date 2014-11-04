#include "../include/CircularLogger.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QApplication>
#include <QHostInfo>
#include <QDebug>


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

	if (m_circularLoggerImplementation != nullptr)
	{
		delete m_circularLoggerImplementation;
	}
}

void CircularLogger::writeRecord(const QString& record)
{
	if (m_circularLoggerImplementation == nullptr)
	{
		qDebug() << "Error writing message " << record << " You should init log";
		Q_ASSERT(false);
		return;
	}
	m_circularLoggerImplementation->pushRecord(record);
}

void CircularLogger::initLog(QString logName, int fileCount, int fileSizeInMB, QString placementPath)
{
	m_circularLoggerImplementation = new CircularLoggerImplementation(logName, fileCount, fileSizeInMB, placementPath);
	m_thread = new QThread(this);
	m_thread->start();
	m_circularLoggerImplementation->moveToThread(m_thread);
}

QString CircularLogger::composeRecord(int type, int category, QString function, QString message)
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

	record += '_';

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

	record += "  ";
	record += message;

	if (!function.isEmpty())
	{
		record += " FN=";
		record += function;
	}
	else
	{
		Q_ASSERT(false);
	}

	return record;
}


CircularLoggerImplementation::CircularLoggerImplementation(QString logName, int fileCount, int fileSizeInMB, QString placementPath) :
	QObject(nullptr),
	m_queue(1000),
	m_logName(logName),
	m_path(placementPath.isEmpty() ? qApp->applicationDirPath() + "/Log" : placementPath),
	m_fileCount(fileCount),
	m_fileSizeLimit(fileSizeInMB)
{
	connect(this, &CircularLoggerImplementation::queueIsNotEmpty, this, &CircularLoggerImplementation::flushQueue, Qt::QueuedConnection);
}

CircularLoggerImplementation::~CircularLoggerImplementation()
{
	if (m_file != nullptr)
	{
		close();
		delete m_stream;
		delete m_file;
	}
}

void CircularLoggerImplementation::pushRecord(const QString& record)
{
	m_queue.push(record);
	emit queueIsNotEmpty();

	qDebug() << record;
}

void CircularLoggerImplementation::close()
{
	if (m_file != nullptr)
	{
		writeLastRecord();
		m_file->close();
	}
}

void CircularLoggerImplementation::flushQueue()
{
	if (m_file == nullptr)
	{
		detectFiles();
	}
	while (!m_queue.isEmpty())
	{
		checkFileSize();
		if (m_queue.isFull())
		{
			m_queue.clear();
			*m_stream << QString("APP_ERR  %1  Log file records queue is full FN=%2")
						 .arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz"))
						 .arg(MESSAGE_POSITION);
		}
		else
		{
			*m_stream << m_queue.pull() << '\n';
		}
	}
	m_stream->flush();
}

void CircularLoggerImplementation::detectFiles()
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

void CircularLoggerImplementation::removeOldFiles()
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

void CircularLoggerImplementation::checkFileSize()
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

int CircularLoggerImplementation::getFileID(int index)
{
	QFile file(fileName(index));
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&file);
	return in.readLine().toInt();
}

QString CircularLoggerImplementation::fileName(int index)
{
	return m_path + '/' + m_logName + '_' + QString("%1").arg(index, 3, 10, QChar('0')) + ".log";
}

void CircularLoggerImplementation::openFile(int index)
{
	if (m_file == nullptr)
	{
		m_file = new QFile(fileName(index));
		m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
		m_stream = new QTextStream(m_file);
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

void CircularLoggerImplementation::writeFirstRecord()
{
	*m_stream << tr("\n# Opened ")
			  << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")
			  << "  by application \""
			  << qApp->applicationName()
			  << "\" host="
			  << QHostInfo::localHostName()
			  << "\n\n";
}

void CircularLoggerImplementation::writeLastRecord()
{
	*m_stream << tr("\n# Closed ")
			  << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")
			  << "  by application \""
			  << qApp->applicationName()
			  << "\" host="
			  << QHostInfo::localHostName()
			  << "\n\n";
	m_stream->flush();
}
