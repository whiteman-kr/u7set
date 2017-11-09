#include "LogFile.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>

namespace Log
{
	LogFileWorker::LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount)
		:m_fileName(fileName),
		  m_path(path),
		  m_maxFileSize(maxFileSize),
		  m_maxFilesCount(maxFilesCount)
	{
		if (m_path.isEmpty() == true)
		{
			QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			m_path = QDir::toNativeSeparators(localAppDataPath);
		}

		qDebug() << "Log path : " << m_path;

		connect(&m_timer, &QTimer::timeout, this, &LogFileWorker::slot_onTimer);
		m_timer.start(500);
	}

	LogFileWorker::~LogFileWorker()
	{
	}

	bool LogFileWorker::write(MessageType type, const QString& text)
	{
		QMutexLocker l(&m_mutex);
		m_queue.push_back(std::make_pair(type, text));

		return true;
	}

	void LogFileWorker::onThreadStarted()
	{
		// Get the number of last log file

		QDir dir(m_path);

		QStringList filters;

		filters << QString("%1_????.log").arg(qAppName());

		QStringList existingFiles = dir.entryList(filters, QDir::Files, QDir::Name);

		m_currentFileNumber = 0;

		if (existingFiles.isEmpty() == false)
		{
			QString lastFile = existingFiles.last();

			lastFile.remove(qAppName());
			lastFile.remove('_');
			lastFile.remove(".log");

			bool ok = false;
			int value = lastFile.toInt(&ok);
			if (ok == true)
			{
				m_currentFileNumber = value;
			}
		}

	}

	void LogFileWorker::onThreadFinished()
	{
		flush();
	}

	void LogFileWorker::slot_onTimer()
	{
		flush();
	}

	QString LogFileWorker::getCurrentFileName() const
	{
		QString fileNumber = QString::number(m_currentFileNumber).rightJustified(4, '0');

		return QString("%1%2%3_%4.log").arg(m_path).arg(QDir::separator()).arg(m_fileName).arg(fileNumber);
	}

	bool LogFileWorker::readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount)
	{
		QFile file(fileName);

		if (file.open(QIODevice::ReadOnly | QIODevice::Text ) == false)
		{
			return false;
		}

		QTextStream logStream(&file);

		// Check the header line length

		QString s = logStream.readLine();
		if (s.isNull() == true)
		{
			return false;
		}
		s = s.trimmed();

		if (s.length() != m_serviceStringLength)
		{
			assert(false);
			return false;
		}

		int serviceLineLength = s.length();

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}

		// Read start time

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		startTime = QDateTime::fromString(s, "dd.MM.yyyy hh:mm:ss");

		if (startTime.isValid() == false)
		{
			assert(false);
			return false;
		}

		// Read end time

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		endTime = QDateTime::fromString(s, "dd.MM.yyyy hh:mm:ss");

		if (endTime.isValid() == false)
		{
			assert(false);
			return false;
		}

		// Read records count

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		bool ok = false;

		recordsCount = s.toInt(&ok);

		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}

	bool LogFileWorker::writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount)
	{
		// Seek to the start
		//
		bool ok = file.seek(0);
		if (ok == false)
		{
			assert(ok);
			return false;
		}

		// Write header
		//

		QString str;

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");

		file.write(str.toUtf8());

		str = tr("Application:\t%1").arg(qAppName());

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("Start Time:\t%1").arg(startTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("End Time:\t%1").arg(endTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("Records Count:\t%1").arg(recordsCount);

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str.clear();

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");

		file.write(str.toUtf8());

		str = "\r\n";

		file.write(str.toUtf8());

		// Seek to the end of the file
		//
		ok = file.seek(file.size());
		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}

	bool LogFileWorker::flush()
	{
		QMutexLocker l(&m_mutex);

		if (m_queue.size() == 0)
		{
			return true;
		}

		QDateTime startTime;
		QDateTime endTime;
		int recordsCount = 0;

		QString fileName = getCurrentFileName();

		// Check current file size and switch to the next file if needed
		{
			QFile file(fileName);

			if (file.size() > m_maxFileSize)
			{
				switchToNextLogFile();

				fileName = getCurrentFileName();
			}
		}

		// Read current file information

		if (readLogFileInfo(fileName, startTime, endTime, recordsCount) == false)
		{
			startTime = QDateTime::currentDateTime();
			endTime = startTime;
		}

		// Open file for writing

		QFile file(fileName);

		if (file.open(QIODevice::Append | QIODevice::Text ) == false)
		{
			return false;
		}

		// Write header data

		endTime = QDateTime::currentDateTime();

		recordsCount += static_cast<int>(m_queue.size());

		writeLogFileInfo(file, startTime, endTime, recordsCount);

		// Write records

		QString timeStr = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

		for (auto it : m_queue)
		{
			const MessageType& type = it.first;

			QString str;

			if (type == MessageType::Text)
			{
				str = it.second + "\r\n";
				file.write(str.toUtf8());
				continue;
			}

			str.append(timeStr);
			str.append('\t');

			switch(type)
			{

			case MessageType::Error:
				str.append("ERR\t");
				break;
			case MessageType::Warning:
				str.append("WRN\t");
				break;
			case MessageType::Message:
				str.append("MSG\t");
				break;
			}

			str.append(it.second);
			str.append("\r\n");

			file.write(str.toUtf8());
		}

		file.close();

		m_queue.clear();

		return true;
	}

	void LogFileWorker::switchToNextLogFile()
	{
		// Delete previous files

		int deleteFileIndex = m_currentFileNumber - m_maxFilesCount - 1;
		if (deleteFileIndex < 0)
		{
			deleteFileIndex += m_maxFileNumber + 1;
		}

		QString deleteFileNumber = QString::number(deleteFileIndex).rightJustified(4, '0');

		QString fileName = QString("%1_%2.log").arg(qAppName()).arg(deleteFileNumber);

		QFile f(m_path + QDir::separator() + fileName);

		bool ok = f.remove();
		if (ok == false)
		{
		}

		m_currentFileNumber++;

		if (m_currentFileNumber > m_maxFileNumber)
		{
			m_currentFileNumber = 0;
		}

	}

	//
	// LogFile
	//

	LogFile::LogFile(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount)
	{

		m_logFileWorker = new LogFileWorker(fileName, path, maxFileSize, maxFilesCount);

		m_logThread.addWorker(m_logFileWorker);
		m_logThread.start();
	}

	LogFile::~LogFile()
	{
		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}
	}

	bool LogFile::writeMessage(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Message, text);
	}

	bool LogFile::writeError(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Text, text);
	}

	bool LogFile::write(MessageType type, const QString& text)
	{
		return m_logFileWorker->write(type, text);
	}

}
