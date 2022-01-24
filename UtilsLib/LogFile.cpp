#ifndef UTILS_LIB_DOMAIN
#error Don't include this file in the project! Link UtilsLib instead.
#endif

#include "LogFile.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QUuid>
#include <QScreen>
#include <QTableView>
#include <QFileDialog>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QKeyEvent>
#include <QClipboard>
#include <QAction>
#include <QShortcut>
#include <QDesktopServices>
#include <QDateTimeEdit>
#include <QScrollBar>
#include <array>
#include "./Ui/UiTools.h"
#include "../CommonLib/Hash.h"

//#define LOGFILE_USE_HEADER	// Uncomment this to use header

namespace Log
{
	//
	// LogFileRecord
	//
	const std::array<QString, 6> messageTypeTextShort{"ERR", "WRN", "MSG", "ALERT", "TXT", "DATA"};

	const QString messageTimeFormat("dd.MM.yyyy hh:mm:ss.zzz");

	QString LogFileRecord::toString(const QString& sessionHashString) const
	{
		if (type == MessageType::Text)
		{
			return QString("%1\t%2\r\n")
					.arg(sessionHashString)
					.arg(QString::fromLocal8Bit(text.c_str()));
		}

		size_t intType = static_cast<int>(type);
		if (intType >= messageTypeTextShort.size())
		{
			assert(false);
			return QString();
		}

		return QString("%1\t%2\t\t%3\t%4\r\n")
				.arg(sessionHashString)
				.arg(QDateTime().fromMSecsSinceEpoch(time).toString(messageTimeFormat))
				.arg(messageTypeTextShort[intType])
				.arg(QString::fromLocal8Bit(text.c_str()));
	}

	bool LogFileRecord::loadFromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash, int* errorCount, int* warningCount)
	{
		const int MAX_STR_LEN = 1024;

		static thread_local std::string str;

		if (str.capacity() < MAX_STR_LEN)
		{
			str.reserve(MAX_STR_LEN);
		}

		const char* bufEnd = buf + bufSize;

		const char* ptr = buf;

		qint64 bufLength = bufSize;

		// Session Hash
		//
		const char* ptrEnd = static_cast<const char*>(memchr(ptr, '\t', bufLength));
		if (ptrEnd == nullptr)
		{
			return false;
		}

		sessionHash = strtoull(ptr, nullptr, 16);

		if (currentSessionHash != 0 && sessionHash != currentSessionHash)
		{
			// Wrong session
			//
			return false;
		}

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;
		if (bufLength > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}

		// Time
		//
		ptrEnd = static_cast<const char*>(memchr(ptr, '\t', bufLength));
		if (ptrEnd == nullptr)
		{
			// No time - do not read this line
			//
			return false;
		}

		qint64 strLen = ptrEnd - ptr;
		if (strLen > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}
		str.assign(ptr, strLen);

		time = QDateTime::fromString(str.c_str(), messageTimeFormat).toMSecsSinceEpoch();

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;
		if (bufLength > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}

		// Skip one tab
		//
		ptrEnd = static_cast<const char*>(memchr(ptr, '\t', bufLength));
		if (ptrEnd == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;
		if (bufLength > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}

		// Type
		//
		type = MessageType::Text;

		ptrEnd = static_cast<const char*>(memchr(ptr, '\t', bufLength));
		if (ptrEnd == nullptr)
		{
			// This is simple text, read it until last \n
			//
			ptrEnd = static_cast<const char*>(memchr(ptr, '\n', bufLength));
			if (ptrEnd == nullptr)
			{
				Q_ASSERT(false);
				return false;
			}

			strLen = ptrEnd - ptr;
			if (strLen > MAX_STR_LEN)
			{
				Q_ASSERT(false);
				return false;
			}

			text.assign(ptr, strLen);
			return true;
		}

		strLen = ptrEnd - ptr;
		if (strLen > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}
		str.assign(ptr, strLen);

		if (strncmp(str.c_str(), "ERR", str.length()) == 0)
		{
			type = MessageType::Error;
			*errorCount++;
		}
		else
		{
			if (strncmp(str.c_str(), "WRN", str.length()) == 0)
			{
				type = MessageType::Warning;
				*warningCount++;
			}
			else
			{
				if (strncmp(str.c_str(), "MSG", str.length()) == 0)
				{
					type = MessageType::Message;
				}
				else
				{
					if (strncmp(str.c_str(), "ALERT", str.length()) == 0)
					{
						type = MessageType::Alert;
					}
					else
					{
						if (strncmp(str.c_str(), "DATA", str.length()) == 0)
						{
							type = MessageType::Data;
						}
						else
						{
							// Unknown type
							//
							assert(false);
							return false;
						}
					}
				}
			}
		}

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;
		if (bufLength > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}

		// Text until end-of-line
		//
		ptrEnd = static_cast<const char*>(memchr(ptr, '\n', bufLength));
		if (ptrEnd == nullptr)
		{
			// \n was not found?
			//
			Q_ASSERT(false);
			return false;
		}

		strLen = ptrEnd - ptr;
		if (strLen > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}
		text.assign(ptr, strLen - 1/*remove last \n*/);
		return true;
	}

	//
	// LogFileWorker
	//
	LogFileWorker::LogFileWorker(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash)
		:m_logName(logName),
		  m_path(path),
		  m_maxFileSize(maxFileSize),
		  m_maxFilesCount(maxFilesCount),
		  m_sessionHash(sessionHash),
		  m_sessionHashString(QString::number(sessionHash, 16).rightJustified(16, '0'))
	{
	}

	LogFileWorker::~LogFileWorker()
	{
		qDebug() << "LogFileWorker::~LogFileWorker, ThreadId " << QThread::currentThreadId();
	}

	bool LogFileWorker::write(MessageType type, const QString& text)
	{
		LogFileRecord r;

		r.time = QDateTime::currentDateTime().toMSecsSinceEpoch();
		r.type = type;
		r.sessionHash = m_sessionHash;
		r.text = text.toLocal8Bit().toStdString();

		{
			QMutexLocker l(&m_queueMutex);
			m_queue.push_back(r);
		}

		emit recordArrived(r);

		return true;
	}

	bool LogFileWorker::writeArray(const QStringList& textArray)
	{
		LogFileRecord r;

		r.time = QDateTime::currentDateTime().toMSecsSinceEpoch();
		r.type = MessageType::Data;
		r.sessionHash = m_sessionHash;
		r.text = textArray.join('\t').toLocal8Bit().toStdString();

		{
			QMutexLocker l(&m_queueMutex);
			m_queue.push_back(r);
		}

		emit recordArrived(r);

		return true;
	}

	void LogFileWorker::read(bool currentSessionOnly)
	{
		emit readStart(currentSessionOnly);
	}

	void LogFileWorker::readFromFile(const QString& fileName)
	{
		emit readLogFromFile(fileName);
	}

	void LogFileWorker::cancelReadFromFile()
	{
		m_cancelReadFromFile = true;
	}

	void LogFileWorker::getLoadedData(std::vector<LogFileRecord>* result)
	{
		QMutexLocker l(&m_readLogMutex);
		result->swap(m_readResult);
	}

	QString LogFileWorker::logName() const
	{
		return m_logName;
	}

	QString LogFileWorker::getCurrentFileName() const
	{
		return getLogFileName(m_currentFileNumber);
	}

	QString LogFileWorker::getLogPath() const
	{
		return m_path;
	}

	quint64 LogFileWorker::sessionHash() const
	{
		return m_sessionHash;
	}

	const QString& LogFileWorker::sessionHashString() const
	{
		return m_sessionHashString;
	}

	void LogFileWorker::onThreadStarted()
	{
		// Initialize path
		//
		if (m_path.isEmpty() == true)
		{
			QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			m_path = QDir::toNativeSeparators(localAppDataPath);
		}

		if (QDir().exists(m_path) == false)
		{
			if (QDir().mkpath(m_path) == false)
			{
				QString errorString = tr("LogFileWorker: can't create path %1").arg(m_path);
				emit writeFailure(errorString);
			}
		}

		// Start timer
		//
		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &LogFileWorker::slot_onTimer);
		m_timer->start(500);

		// Get the number of last log file
		//
		QDir dir(m_path);

		QStringList filters;

		filters << QString("%1_????.log").arg(m_logName);

		QStringList existingFiles = dir.entryList(filters, QDir::Files, QDir::Name);

		m_currentFileNumber = 0;

		if (existingFiles.isEmpty() == false)
		{
			QString lastFile = existingFiles.last();

			lastFile.remove(m_logName);
			lastFile.remove('_');
			lastFile.remove(".log");

			bool ok = false;
			int value = lastFile.toInt(&ok);
			if (ok == true)
			{
				m_currentFileNumber = value;
			}
		}

		// Connect load event to load slot

		connect(this, &LogFileWorker::readStart, this, &LogFileWorker::slot_load);
		connect(this, &LogFileWorker::readLogFromFile, this, &LogFileWorker::slot_loadFromFile);

		// Create shared memory to lock file writing
		//
		m_sharedMemory = std::make_unique<QSharedMemory>();
		m_sharedMemory->setKey(qAppName() + m_logName);

		bool ok = m_sharedMemory->create(sizeof(bool));

		if (ok == true)
		{
			// If it was created then it must be initialized
			//
			m_sharedMemory->lock();

			bool locked = false;
			memcpy(m_sharedMemory->data(), &locked, sizeof(locked));

			m_sharedMemory->unlock();
		}
		else
		{
			if (m_sharedMemory->error() == QSharedMemory::SharedMemoryError::AlreadyExists)
			{
				bool aok = m_sharedMemory->attach();
				Q_ASSERT(aok);

				if (aok == false)
				{
					QString errorString = tr("LogFileWorker: can't attach to QSharedMemory");
					emit writeFailure(errorString);
				}
			}
			else
			{
				Q_ASSERT(false);

				QString errorString = tr("LogFileWorker: can't create QSharedMemory: %1").arg(m_sharedMemory->errorString());
				emit writeFailure(errorString);
			}
		}
	}

	void LogFileWorker::onThreadFinished()
	{
		m_timer->stop();

		QString errorString;

		if (flush(&errorString) == false)
		{
			emit writeFailure(errorString);
		}
	}

	QString LogFileWorker::getLogFileName(int index) const
	{
		QString fileNumber = QString::number(index).rightJustified(4, '0');

		return QString("%1%2%3_%4.log").arg(m_path).arg(QDir::separator()).arg(m_logName).arg(fileNumber);
	}

	bool LogFileWorker::readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount)
	{
		QFile file(fileName);

		if (file.exists() == false)
		{
			return false;
		}

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

		qsizetype serviceLineLength = s.length();

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
		file.write(str.toLocal8Bit());

		str = tr("Application:\t%1").arg(qAppName());
		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");
		file.write(str.toLocal8Bit());

		str = tr("Start Time:\t%1").arg(startTime.toString("dd.MM.yyyy hh:mm:ss"));
		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");
		file.write(str.toLocal8Bit());

		str = tr("End Time:\t%1").arg(endTime.toString("dd.MM.yyyy hh:mm:ss"));
		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");
		file.write(str.toLocal8Bit());

		str = tr("Records Count:\t%1").arg(recordsCount);
		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");
		file.write(str.toLocal8Bit());

		str.clear();

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");
		file.write(str.toLocal8Bit());

		str = "\r\n";
		file.write(str.toLocal8Bit());

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

	bool LogFileWorker::lockShared(bool lock, bool* alreadyLocked)
	{
		// Read currentLocked
		//
		bool currentLocked = false;

		bool lok = m_sharedMemory->lock();
		if (lok == false)
		{
			Q_ASSERT(lok);
			return false;
		}

		memcpy(&currentLocked, m_sharedMemory->data(), sizeof(currentLocked));

		bool uok = m_sharedMemory->unlock();
		if (uok == false)
		{
			Q_ASSERT(uok);
			return false;
		}

		if (alreadyLocked != nullptr)
		{
			*alreadyLocked = currentLocked;
		}

		// If already locked or unlocked, return true
		//
		if (lock == currentLocked)
		{
			return true;
		}

		// Write lock
		//
		lok = m_sharedMemory->lock();
		if (lok == false)
		{
			Q_ASSERT(lok);
			return false;
		}

		memcpy(m_sharedMemory->data(), &lock, sizeof(lock));

		uok = m_sharedMemory->unlock();
		if (uok == false)
		{
			Q_ASSERT(uok);
			return false;
		}

		return true;
	}

	bool LogFileWorker::flush(QString* errorString)
	{
		if (errorString == nullptr)
		{
			assert(errorString);
			return false;
		}

		errorString->clear();


		m_queueMutex.lock();
		std::vector<LogFileRecord> queueCopy{std::move(m_queue)};
		m_queue = {};
		m_queue.reserve(64);
		m_queueMutex.unlock();

		if (queueCopy.empty() == true)
		{
			return true;
		}

		QString fileName = getCurrentFileName();

		// Try to lock log file for writing
		//
		bool alreadyLocked = false;

		bool lok = lockShared(true, &alreadyLocked);

		if (lok == false)
		{
			*errorString = tr("Error locking log file %1 by QSharedMemory.").arg(fileName);
			return false;
		}

		if (alreadyLocked == true)
		{
			const int maxLogQueueSize = 1000;

			if (queueCopy.size() > maxLogQueueSize)
			{
				*errorString = tr("Log file %1 is locked by another instance and queue size exceeds maximum, queue cleared.").arg(fileName);
				return false;
			}
			else
			{
				return true;
			}
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr, [&](int *) { lockShared(false, nullptr); });

		// Check current file size and switch to the next file if needed
		//
		{
			QFile file(fileName);

			if (file.size() > m_maxFileSize)
			{
				if (switchToNextLogFile(errorString) == false)
				{
					return false;
				}

				fileName = getCurrentFileName();
			}
		}


#ifdef LOGFILE_USE_HEADER
		// Read current file information

		QDateTime startTime;
		QDateTime endTime;
		int recordsCount = 0;

		if (readLogFileInfo(fileName, startTime, endTime, recordsCount) == false)
		{
			startTime = QDateTime::currentDateTime();
			endTime = startTime;
		}
#endif

		// Open file for writing

		QFile file(fileName);

		if (file.open(QIODevice::Append | QIODevice::Text ) == false)
		{
			*errorString = tr("Log file %1 open error: %2").arg(fileName).arg(file.errorString());
			return false;
		}

		// Write header data

#ifdef LOGFILE_USE_HEADER
		endTime = QDateTime::currentDateTime();

		recordsCount += static_cast<int>(queueCopy.size());

		writeLogFileInfo(file, startTime, endTime, recordsCount);
#endif

		// Write records
		//
		for (const LogFileRecord& record : queueCopy)
		{
			if (file.write(record.toString(m_sessionHashString).toLocal8Bit()) == -1)
			{
				*errorString = tr("Log file %1 write error: %2").arg(fileName).arg(file.errorString());
				file.close();
				return false;
			}
		}

		file.close();

		return true;
	}

	bool LogFileWorker::switchToNextLogFile(QString* errorString)
	{
		if (errorString == nullptr)
		{
			assert(errorString);
			return false;
		}

		// If current file is less than max count, switch to the next file
		//
		if (m_currentFileNumber < m_maxFilesCount - 1)
		{
			m_currentFileNumber++;
			return true;
		}

		// Delete file number 0
		//
		{
			QString fileName = getLogFileName(0);

			QFile f(fileName);

			if (f.exists() == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, file %1 does not exist.").arg(fileName);
				return false;
			}

			if (f.remove() == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, can't remove file %1.").arg(fileName);
				return false;
			}
		}

		// Rename others

		for (int i = 1; i < m_maxFilesCount; i++)
		{
			QString fileNameOld = getLogFileName(i);
			QString fileNameNew = getLogFileName(i - 1);

			QFile f(fileNameOld);

			if (f.rename(fileNameNew) == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, can't rename file %1 -> %2.").arg(fileNameOld).arg(fileNameNew);
				return false;
			}
		}

		return true;
	}

	bool LogFileWorker::readFileRecords(const QString& fileName, bool currentSessionOnly, std::vector<LogFileRecord>* result, int* errorCount, int* warningCount)
	{
		if (result == nullptr || errorCount == nullptr || warningCount == nullptr)
		{
			assert(result);
			assert(errorCount);
			assert(warningCount);
			return false;
		}

		QFile f(fileName);

		if (f.exists() == false)
		{
			return true;
		}

		if (f.open(QFile::ReadOnly) == false)
		{
			return false;
		}

#ifdef LOGFILE_USE_HEADER
		// Read and skip header
		//
		Q_ASSERT(false);

		const int headerLinesCount = 7;

		for (int i = 0; i < headerLinesCount; i++)
		{
			if (stream.readLine().isNull() == true)
			{
				return false;
			}
		}
#endif
		LogFileRecord record;

		const QByteArray ba =  f.readAll();

		const char* ptr = ba.data();

		const char* endPtr = ptr + ba.length();

		qint64 numLines = 0;

		const quint64 sessionHash = currentSessionOnly == true ? m_sessionHash : 0;

		while (ptr != nullptr && ptr != endPtr)
		{
			const char* str = ptr;

			ptr = static_cast<const char*>(memchr(ptr, '\n', endPtr - ptr));

			if (ptr != nullptr)
			{
				qint64 len = ptr - str + 1;	// Length of the string INCLUDING '\n'

				if (record.loadFromString(str, len, sessionHash, errorCount, warningCount) == true)
				{
					result->push_back(record);
				}

				numLines++;
				ptr++;
			}

			if (m_cancelReadFromFile == true)
			{
				break;
			}
		}

		return true;
	}


	void LogFileWorker::slot_load(bool currentSessionOnly)
	{
		// Try to lock the log
		//
		bool lockSuccess = false;

		for (int i = 0; i < 100; i++)
		{
			bool alreadyLocked = false;

			bool lok = lockShared(true, &alreadyLocked);

			if (lok == false)
			{
				Q_ASSERT(lok);

				emit readComplete();
				return;
			}

			if (alreadyLocked == false)
			{
				lockSuccess = true;
				break;
			}

			QThread::msleep(10);
		}

		if (lockSuccess == false)
		{
			emit readComplete();
			return;
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr, [&](int*) { lockShared(false, nullptr); });

		// Read the log
		//
		std::vector<LogFileRecord> readResult;
		readResult.reserve(64000);
		int errorCount = 0;
		int warningCount = 0;

		m_cancelReadFromFile = false;

		for (int i = 0; i < m_maxFilesCount; i++)
		{
			emit readInProgress(getLogFileName(i), static_cast<int>(readResult.size()));
			readFileRecords(getLogFileName(i), currentSessionOnly, &readResult, &errorCount, &warningCount);

			if (m_cancelReadFromFile == true)
			{
				break;
			}
		}

		{
			QMutexLocker l(&m_readLogMutex);
			m_readResult = std::move(readResult);
			m_errorCount = errorCount;
			m_warningCount = warningCount;
		}

		emit readComplete();
	}

	void LogFileWorker::slot_loadFromFile(QString fileName)
	{
		// Try to lock the log
		//
		bool lockSuccess = false;

		for (int i = 0; i < 100; i++)
		{
			bool alreadyLocked = false;

			bool lok = lockShared(true, &alreadyLocked);

			if (lok == false)
			{
				Q_ASSERT(lok);

				emit readFromFileComplete(fileName);
				return;
			}

			if (alreadyLocked == false)
			{
				lockSuccess = true;
				break;
			}

			QThread::msleep(10);
		}

		if (lockSuccess == false)
		{
			emit readFromFileComplete(fileName);
			return;
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr, [&](int*) { lockShared(false, nullptr); });

		// Read the log
		//
		std::vector<LogFileRecord> readResult;
		readResult.reserve(64000);
		int errorCount = 0;
		int warningCount = 0;

		m_cancelReadFromFile = false;

		emit readInProgress(fileName, static_cast<int>(readResult.size()));
		readFileRecords(fileName, false/*currentSessionOnly*/, &readResult, &errorCount, &warningCount);

		{
			QMutexLocker l(&m_readLogMutex);
			m_readResult = std::move(readResult);
			m_errorCount = errorCount;
			m_warningCount = warningCount;
		}

		emit readFromFileComplete(fileName);
	}

	void LogFileWorker::slot_onTimer()
	{
		QString errorString;

		if (flush(&errorString) == false)
		{
			emit writeFailure(errorString);
		}
	}


	//
	// LogRecordModel
	//
	LogRecordModel::LogRecordModel(bool showTypeColumn, std::vector<std::pair<QString, double> > headerTitles):
		m_showTypeColumn(showTypeColumn)
	{
		int c = 0;

		double usedWidth = 0;

		m_columnsNames << tr("Time");
		m_columnsWidthPercent.push_back(0.20);
		usedWidth += 0.20;
		m_columnTime = c++;

		if (m_showTypeColumn == true)
		{
			m_columnsNames << tr("Type");
			m_columnsWidthPercent.push_back(0.05);
			usedWidth += 0.05;
			m_columnType = c++;
		}

		if (headerTitles.size() == 0)
		{
			m_columnsNames << tr("Message");
			m_columnsWidthPercent.push_back(1 - usedWidth);
			m_columnText = c++;
		}
		else
		{
			int count = static_cast<int>(headerTitles.size());

			double totalWidth = (1 - usedWidth);

			for (int i = 0; i < count; i++)
			{
				const QString& s = headerTitles[i].first;

				m_columnsNames << s;
				m_columnsWidthPercent.push_back(totalWidth * headerTitles[i].second);
			}

			m_columnText = c++;
		}
	}

	LogRecordModel::~LogRecordModel()
	{
	}

	void LogRecordModel::setRecords(std::vector<LogFileRecord>* records)
	{
		m_records = std::move(*records);

		fillRecords();
	}

	void LogRecordModel::addRecord(const LogFileRecord& record)
	{
		m_records.push_back(record);

		if (processRecordFilter(record) == true)
		{
			int index = static_cast<int>(m_filteredRecordsIndex.size());

			beginInsertRows(QModelIndex(), index, index);

			m_filteredRecordsIndex.push_back(static_cast<int>(m_records.size() - 1));

			if (record.type == MessageType::Error)
			{
				m_errorCount++;
			}
			else
			{
				if (record.type == MessageType::Warning)
				{
					m_warningCount++;
				}
			}

			insertRows(index, 1);

			endInsertRows();
		}
	}

	int LogRecordModel::recordsCount() const
	{
		return static_cast<int>(m_records.size());

	}

	int LogRecordModel::filterRecordTypeMask() const
	{
		return m_filterRecordTypeMask;
	}

	void LogRecordModel::setFilterRecordTypeMask(int value)
	{
		m_filterRecordTypeMask = value;
	}

	QString LogRecordModel::filterText() const
	{
		return QString::fromLocal8Bit(m_filterText.c_str());
	}

	void LogRecordModel::setFilterText(const QString& value)
	{
		m_filterText = value.toLocal8Bit().toStdString();
	}

	qint64 LogRecordModel::filterTimeFrom() const
	{
		return m_filterTimeFrom;
	}

	void LogRecordModel::setFilterTimeFrom(qint64 value)
	{
		m_filterTimeFrom = value;
	}

	qint64 LogRecordModel::filterTimeTo() const
	{
		return m_filterTimeTo;
	}

	void LogRecordModel::setFilterTimeTo(qint64 value)
	{
		m_filterTimeTo = value;
	}

	void LogRecordModel::fillRecords()
	{
		m_errorCount = 0;
		m_warningCount = 0;

		// Remove data from the model
		//
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			removeRows(0, rowCount());
			m_filteredRecordsIndex.clear();
			endRemoveRows();
		}

		// Process filters
		//
		int count = static_cast<int>(m_records.size());

		m_filteredRecordsIndex.reserve(count);

		for (int i = 0; i < count; i++)
		{
			const LogFileRecord& rec = m_records[i];

			if (processRecordFilter(rec) == true)
			{
				m_filteredRecordsIndex.push_back(i);

				if (rec.type == MessageType::Error)
				{
					m_errorCount++;
				}
				else
				{
					if (rec.type == MessageType::Warning)
					{
						m_warningCount++;
					}
				}
			}
		}

		// Set data to the model
		//
		if (m_filteredRecordsIndex.empty() == false)
		{
			int filterRecordsCount = static_cast<int>(m_filteredRecordsIndex.size());

			beginInsertRows(QModelIndex(), 0, filterRecordsCount - 1);

			insertRows(0, static_cast<int>(filterRecordsCount));

			endInsertRows();
		}
	}

	void LogRecordModel::getFilteredRecords(std::vector<LogFileRecord>* result) const
	{
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return;
		}

		std::vector<LogFileRecord> res;

		int count = static_cast<int>(m_filteredRecordsIndex.size());
		for (int i = 0; i < count; i++)
		{
			res.push_back(m_records[m_filteredRecordsIndex[i]]);
		}

		*result = std::move(res);
		return;
	}

	bool LogRecordModel::processRecordFilter(const LogFileRecord& record) const
	{
		if (m_filterRecordTypeMask != MessageType::All)
		{
			if ((m_filterRecordTypeMask & (1 << record.type)) == 0)
			{
				return false;
			}
		}

		if (m_filterText.empty() == false)
		{
			if (record.text.find(m_filterText) == std::string::npos)
			{
				return false;
			}
		}

		if (m_filterTimeFrom != -1 && m_filterTimeTo != -1)
		{
			if (record.time < m_filterTimeFrom || record.time > m_filterTimeTo)
			{
				return false;
			}
		}

		return true;
	}

	double LogRecordModel::columnWidthPercent(size_t index)
	{
		if (index >= m_columnsWidthPercent.size())
		{
			assert(false);
			return 100;
		}

		return m_columnsWidthPercent[index];
	}

	int LogRecordModel::errorCount() const
	{
		return m_errorCount;
	}

	int LogRecordModel::warningCount() const
	{
		return m_warningCount;
	}

	int LogRecordModel::searchIssue(int startRow, bool forward) const
	{
		int row = startRow;

		do
		{
			if (forward == true)
			{
				row++;
				if (row >= rowCount())	// Loop to start
				{
					row = 0;
				}
			}
			else
			{
				row--;
				if (row < 0)	// Loop to end
				{
					row = rowCount() - 1;
				}
			}

			const LogFileRecord& rec = m_records[m_filteredRecordsIndex[row]];

			if (rec.type == MessageType::Warning || rec.type == MessageType::Error)
			{
				return row;
			}
		}while (row != startRow);

		return -1;
	}

	int LogRecordModel::searchRecord(int startRow, const std::string& text) const
	{
		if (text.empty() == true)
		{
			return -1;
		}

		int row = startRow;

		do
		{
			row++;
			if (row >= rowCount())	// Loop to start
			{
				row = 0;
			}

			const LogFileRecord& rec = m_records[m_filteredRecordsIndex[row]];



			if (rec.text.find(text) != std::string::npos)
			{
				return row;
			}
		}while (row != startRow);

		return -1;
	}


	int LogRecordModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_filteredRecordsIndex.size());
	}

	int LogRecordModel::columnCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_columnsNames.size());
	}

	QModelIndex LogRecordModel::index(int row, int column, const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return createIndex(row, column);
	}

	QVariant LogRecordModel::data(const QModelIndex& index, int role) const
	{
		size_t column = index.column();
		if (column >= static_cast<size_t>(m_columnsNames.size()))
		{
			assert(false);
			return {};
		}

		size_t row = index.row();
		if (row >= m_filteredRecordsIndex.size())
		{
			assert(false);
			return QVariant();
		}

		size_t recordIndex = m_filteredRecordsIndex[row];
		if (recordIndex >= m_records.size())
		{
			Q_ASSERT(recordIndex < m_records.size());
			return {};
		}

		const LogFileRecord& record = m_records[recordIndex];

		if (role == Qt::DisplayRole)
		{
			int displayIndex = static_cast<int>(column);
			if (displayIndex == m_columnTime)
			{
				return QDateTime().fromMSecsSinceEpoch(record.time).toString(messageTimeFormat);
			}

			if (displayIndex == m_columnType)
			{
				size_t intType = static_cast<size_t>(record.type);
				if (intType >= messageTypeTextShort.size())
				{
					assert(false);
					return QString();
				}

				return messageTypeTextShort[intType];
			}

			if (displayIndex >= m_columnText)
			{
				if (record.type == MessageType::Data)
				{
					int textColumnNo = displayIndex - m_columnText;

					if (textColumnNo < 0)
					{
						assert(false);
						return QVariant();
					}

					QStringList textArray = QString::fromLocal8Bit(record.text.c_str()).split('\t');

					if (textColumnNo < textArray.size())
					{
						return textArray[textColumnNo];
					}
				}
				else
				{
					return QString::fromLocal8Bit(record.text.c_str());
				}
			}
		}

		if (role == Qt::ForegroundRole)
		{
			switch (record.type)
			{
			case MessageType::Error:
				return QBrush{qRgb(0xE0, 0x33, 0x33)};
			case MessageType::Alert:
				return QBrush{qRgb(0xE0, 0x33, 0x33)};
			case MessageType::Warning:
				return QBrush{qRgb(0xF8, 0x72, 0x17)};
			default:
				return {};
			}
		}

		return QVariant();
	}

	QModelIndex LogRecordModel::parent(const QModelIndex& index) const
	{
		Q_UNUSED(index);
		return QModelIndex();
	}

	QVariant LogRecordModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			if (section < 0 || section >= m_columnsNames.size())
			{
				assert(false);
				return QVariant();
			}

			return m_columnsNames.at(section);
		}

		return QVariant();
	}

	//
	// SelectionControlDelegate
	//
	SelectionControlDelegate::SelectionControlDelegate(QObject* parent, LogRecordModel* model) :
		QStyledItemDelegate(parent),
		m_model(model)
	{
	}

	void SelectionControlDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);

		// Set background color for selected item (by default it is displayed by white)
		//
		if (option->state & QStyle::State_Selected)
		{
			QBrush br = m_model->data(index, Qt::ForegroundRole).value<QBrush>();
			option->palette.setColor(QPalette::HighlightedText, br.color());
		}
	}

	//
	// LogFileProgressDialog
	//
	LogFileProgressDialog::LogFileProgressDialog(QWidget* parent):
		QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout(this);

		setMinimumSize(350, 100);

		m_label = new QLabel("Loading...");
		m_label->setAlignment(Qt::AlignCenter);
		mainLayout->addWidget(m_label);

		QHBoxLayout* bl = new QHBoxLayout();

		bl->addStretch();

		QPushButton* b = new QPushButton(tr("Cancel"));
		bl->addWidget(b);
		connect(b, &QPushButton::clicked, this, &LogFileProgressDialog::cancelRead);

		bl->addStretch();

		mainLayout->addLayout(bl);

		setLayout(mainLayout);
	}

	LogFileProgressDialog::~LogFileProgressDialog()
	{

	}

	void LogFileProgressDialog::readInProgress(QString fileName, int recordsRead)
	{
		m_label->setText(tr("Loading file: %1\n\nRecords loaded: %2").arg(QFileInfo(fileName).fileName()).arg(recordsRead));
	}

	void LogFileProgressDialog::readComplete()
	{
		QDialog::reject();
	}

	//
	// DialogTimeFilter
	//

	DialogTimeFilter::DialogTimeFilter(qint64 filterTimeFrom, qint64 filterTimeTo, QWidget* parent):
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		m_filterTimeFrom(filterTimeFrom),
		m_filterTimeTo(filterTimeTo)
	{
		setWindowTitle(tr("Time Filter"));

		QVBoxLayout* mainLayout = new QVBoxLayout();

		mainLayout->addWidget(new QLabel("Start Time:"));
		m_timeFromEdit = new QDateTimeEdit(this);
		m_timeFromEdit->setDisplayFormat(messageTimeFormat);
		mainLayout->addWidget(m_timeFromEdit);

		mainLayout->addWidget(new QLabel("End Time:"));
		m_timeToEdit = new QDateTimeEdit(this);
		m_timeToEdit->setDisplayFormat(messageTimeFormat);
		mainLayout->addWidget(m_timeToEdit);

		if (m_filterTimeFrom != -1 && m_filterTimeTo != -1)
		{
			m_timeFromEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(m_filterTimeFrom));
			m_timeToEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(m_filterTimeTo));
		}
		else
		{
			QDateTime now = QDateTime::currentDateTime();
			m_timeFromEdit->setDateTime(now);
			m_timeToEdit->setDateTime(now);
		}

		QHBoxLayout* hb = new QHBoxLayout();
		hb->addStretch();

		QPushButton* b = new QPushButton(tr("Set"));
		connect(b, &QPushButton::clicked, this, &DialogTimeFilter::accept);
		hb->addWidget(b);

		b = new QPushButton(tr("Reset"));
		connect(b, &QPushButton::clicked, [this](){
			m_filterTimeFrom = -1;
			m_filterTimeTo = -1;
			QDialog::accept();
		});
		hb->addWidget(b);

		b = new QPushButton(tr("Cancel"));
		connect(b, &QPushButton::clicked, this, &DialogTimeFilter::reject);
		hb->addWidget(b);

		mainLayout->addLayout(hb);

		setLayout(mainLayout);

		return;
	}

	DialogTimeFilter::~DialogTimeFilter()
	{

	}

	qint64 DialogTimeFilter::filterTimeFrom() const
	{
		return m_filterTimeFrom;
	}

	qint64 DialogTimeFilter::filterTimeTo() const
	{
		return m_filterTimeTo;
	}

	void DialogTimeFilter::accept()
	{
		m_filterTimeFrom = m_timeFromEdit->dateTime().toMSecsSinceEpoch();
		m_filterTimeTo = m_timeToEdit->dateTime().toMSecsSinceEpoch();

		if (m_filterTimeFrom >= m_filterTimeTo)
		{
			m_timeFromEdit->setFocus();
			QMessageBox::critical(this, qAppName(), tr("Start Time should be earlier than End Time!"));
			return;
		}

		QDialog::accept();
		return;
	}


	//
	// LogTableView
	//

	void LogTableView::keyPressEvent(QKeyEvent *event)
	{
		if (event->key() == Qt::Key_F6 || event->key() == Qt::Key_F3)
		{
			return;
		}

		if (selectedIndexes().size() > 0)
		{
			if ((event->modifiers() & Qt::ControlModifier) != 0 && event->key() == Qt::Key_C)
			{
				emit copyKeyPressed();
				return;
			}
		}
		else
		{
			if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
			{
				return;
			}
		}

		if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
			event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
			event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
		{
			emit turnOffAutoscroll();
		}

		QTableView::keyPressEvent(event);
		return;
	}

	void LogTableView::wheelEvent(QWheelEvent* event)
	{
		emit turnOffAutoscroll();

		QTableView::wheelEvent(event);
		return;
	}


	//
	// LogFileDialog
	//

	LogFileDialog::LogFileDialog(LogFileWorker* worker, QWidget* parent, bool useMessageType, bool headerVisible, const std::vector<std::pair<QString, double>>& headerTitles) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
		m_worker(worker),
		m_model(useMessageType, headerTitles),
		m_progressDialog(this)
	{
		setAttribute(Qt::WA_DeleteOnClose);

		setWindowTitle(tr("Log View - %1").arg(worker->logName()));

		connect(m_worker, &LogFileWorker::readInProgress, &m_progressDialog, &LogFileProgressDialog::readInProgress);

		connect(m_worker, &LogFileWorker::readComplete, &m_progressDialog, &LogFileProgressDialog::readComplete);
		connect(m_worker, &LogFileWorker::readFromFileComplete, &m_progressDialog, &LogFileProgressDialog::readComplete);

		connect(&m_progressDialog, &LogFileProgressDialog::cancelRead, [this](){m_worker->cancelReadFromFile();});

		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		QHBoxLayout* topLayout = new QHBoxLayout();
		mainLayout->addLayout(topLayout);

		if (useMessageType == true)
		{
			topLayout->addWidget(new QLabel("Type:"));

			m_recordTypeCombo = new QComboBox();
			m_recordTypeCombo->addItem(tr("All Messages"), MessageType::All);
			m_recordTypeCombo->addItem(tr("Errors"), 1 << MessageType::Error);
			m_recordTypeCombo->addItem(tr("Warnings"), 1 << MessageType::Warning);
			m_recordTypeCombo->addItem(tr("Errors and Warnings"), (1 << (MessageType::Error)) | (1 << (MessageType::Warning)));
			m_recordTypeCombo->addItem(tr("Alerts"), 1 << MessageType::Alert);
			m_recordTypeCombo->addItem(tr("Messages"), 1 << MessageType::Message);
			m_recordTypeCombo->addItem(tr("Text"), 1 << MessageType::Text);
			m_recordTypeCombo->setCurrentIndex(0);
			connect(m_recordTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogFileDialog::onTypeComboIndexChanged);

			topLayout->addWidget(m_recordTypeCombo);
		}

		// --
		//
		m_timeFilterButton = new QPushButton(tr("Time Filter"));
		connect(m_timeFilterButton, &QPushButton::clicked, this, &LogFileDialog::onTimeFilter);
		topLayout->addWidget(m_timeFilterButton);

		topLayout->addStretch();

		// --
		//
		m_filterLineEdit = new QLineEdit();
		m_filterLineEdit->setPlaceholderText(tr("Search/Filter Text"));
		m_filterLineEdit->setClearButtonEnabled(true);
		topLayout->addWidget(m_filterLineEdit);
		connect(m_filterLineEdit, &QLineEdit::returnPressed, this, &LogFileDialog::onFilter);
		connect(m_filterLineEdit, &QLineEdit::textEdited, this, &LogFileDialog::onFilterEdited);

		QString text = QString(50, 'a');
		QFontMetrics fm(m_filterLineEdit->font());
		int pixelsWide = fm.boundingRect(text).width();
		m_filterLineEdit->setFixedWidth(pixelsWide);

		// --
		//
		QShortcut* shF3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
		connect(shF3, &QShortcut::activated, this, &LogFileDialog::onSearch);

		// --
		//
		m_search = new QPushButton(tr("Search <F3>"));
		m_search->setEnabled(false);
		m_search->setDefault(true);
		topLayout->addWidget(m_search);

		connect(m_search, &QPushButton::clicked, this, &LogFileDialog::onSearch);

		// --
		//
		m_filter = new QPushButton(tr("Filter"));
		m_filter->setEnabled(false);
		topLayout->addWidget(m_filter);

		connect(m_filter, &QPushButton::clicked, this, &LogFileDialog::onFilter);

		// --
		//
		if (useMessageType == true)
		{
			m_prevIssue = new QPushButton(tr("Prev Issue <Shift+F6>"));
			connect(m_prevIssue, &QPushButton::clicked, this, &LogFileDialog::onPrevIssue);
			topLayout->addWidget(m_prevIssue);

			QShortcut* shShiftF6 = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F6), this);
			connect(shShiftF6, &QShortcut::activated, this, &LogFileDialog::onPrevIssue);

			m_nextIssue = new QPushButton(tr("Next Issue <F6>"));
			connect(m_nextIssue, &QPushButton::clicked, this, &LogFileDialog::onNextIssue);
			topLayout->addWidget(m_nextIssue);

			QShortcut* shF6 = new QShortcut(QKeySequence(Qt::Key_F6), this);
			connect(shF6, &QShortcut::activated, this, &LogFileDialog::onNextIssue);
		}

		// --
		//
		topLayout->addStretch();

		// --
		//
		m_allSessions = new QPushButton(tr("All Sessions"));
		m_allSessions->setCheckable(true);
		m_allSessions->setChecked(false);
		topLayout->addWidget(m_allSessions);
		connect(m_allSessions, &QPushButton::clicked, this, &LogFileDialog::onAllSessionsClicked);

		// --
		//
		m_autoScroll = new QPushButton(tr("Auto Scroll"));
		m_autoScroll->setCheckable(true);
		topLayout->addWidget(m_autoScroll);

		// --
		//
		m_table = new LogTableView();
		m_table->setModel(&m_model);

		m_table->verticalHeader()->hide();
		m_table->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_table->setItemDelegate(new SelectionControlDelegate(this, &m_model));

		if (headerVisible == false)
		{
			m_table->horizontalHeader()->hide();
			m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
			m_table->horizontalHeader()->setStretchLastSection(true);
		}
		else
		{
			m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		}

		m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

		m_table->setSortingEnabled(false);

		connect(m_table, &LogTableView::copyKeyPressed, this, &LogFileDialog::onCellCopyKeyPressed);
		connect(m_table->verticalScrollBar(), &QScrollBar::sliderMoved, [this](){
			turnOffAutoscroll();
		});
		connect(m_table, &LogTableView::turnOffAutoscroll, [this](){
			turnOffAutoscroll();
		});

		// --
		//
		mainLayout->addWidget(m_table);

		// --
		//
		QHBoxLayout* bottomLayout = new QHBoxLayout();

		m_counterLabel = new QLabel();
		bottomLayout->addWidget(m_counterLabel);

		m_logPathLabel = new QLabel();
		m_logPathLabel->setTextFormat(Qt::RichText);
		m_logPathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_worker->getCurrentFileName()));
		connect(m_logPathLabel, &QLabel::linkActivated, this, &LogFileDialog::onLinkActivated);

		bottomLayout->addWidget(m_logPathLabel);

		bottomLayout->addStretch();

		m_clear = new QPushButton(tr("Clear"));
		connect(m_clear, &QPushButton::clicked, this, &LogFileDialog::onClear);
		bottomLayout->addWidget(m_clear);

		m_load = new QPushButton(tr("Load..."));
		connect(m_load, &QPushButton::clicked, this, &LogFileDialog::onLoad);
		bottomLayout->addWidget(m_load);

		m_export = new QPushButton(tr("Export..."));
		connect(m_export, &QPushButton::clicked, this, &LogFileDialog::onExport);
		bottomLayout->addWidget(m_export);

		mainLayout->addLayout(bottomLayout);

		// --
		//
		connect(m_worker, &LogFileWorker::readComplete, this, &LogFileDialog::onLoadComplete);
		connect(m_worker, &LogFileWorker::readFromFileComplete, this, &LogFileDialog::onLoadFromFileComplete);
		connect(m_worker, &LogFileWorker::recordArrived, this, &LogFileDialog::onRecordArrived, Qt::QueuedConnection);

		// Restore settings
		//
		QSettings s;

		QPoint windowPos = s.value("LogFileDialog/windowPos", QPoint(-1, -1)).toPoint();
		QByteArray windowGeometry = s.value("LogFileDialog/windowGeometry").toByteArray();

		if (windowPos.x() != -1 && windowPos.y() != -1)
		{
			move(windowPos);
		}
		else
		{
			QRect desktopRect = this->screen()->availableGeometry();
			int x = (desktopRect.width() - width()) / 2;
			int y = (desktopRect.height() - height()) / 2;
			move(QPoint(x, y));
		}

		if (windowGeometry.isEmpty() == false)
		{
			restoreGeometry(windowGeometry);
		}

		bool autoScroll = s.value("LogFileDialog/autoScroll", true).toBool();
		m_autoScroll->setChecked(autoScroll);

		// Read existing log

		m_worker->read(true/*currentSessionOnly*/);

		enableControls(false);

		m_progressDialog.exec();
	}

	LogFileDialog::~LogFileDialog()
	{
		QSettings s;

		s.setValue("LogFileDialog/windowPos", pos());
		s.setValue("LogFileDialog/windowGeometry", saveGeometry());

		if (m_autoScroll != nullptr)
		{
			bool autoScroll = m_autoScroll->isChecked();
			s.setValue("LogFileDialog/autoScroll", autoScroll);
		}
	}

	void LogFileDialog::showEvent(QShowEvent* event)
	{
		Q_UNUSED(event);

		if (m_firstShow == true)
		{
			m_firstShow = false;
			adjustColumnsWidth();
		}
	}

	void LogFileDialog::resizeEvent(QResizeEvent* event)
	{
		Q_UNUSED(event);
		adjustColumnsWidth();
	}

	void LogFileDialog::enableControls(bool enable)
	{
		if (m_recordTypeCombo != nullptr)
		{
			m_recordTypeCombo->setEnabled(enable);
		}

		m_timeFilterButton->setEnabled(enable);
		m_filterLineEdit->setEnabled(enable);
		m_allSessions->setEnabled(enable);
		m_autoScroll->setEnabled(enable);

		if (m_prevIssue != nullptr)
		{
			m_prevIssue->setEnabled(enable);
		}
		if (m_nextIssue != nullptr)
		{
			m_nextIssue->setEnabled(enable);
		}

		m_clear->setEnabled(enable);
		m_export->setEnabled(enable);

		m_search->setEnabled(m_filterLineEdit->text().isEmpty() == false);
		m_filter->setEnabled(m_filterLineEdit->text().isEmpty() == false);
	}

	void LogFileDialog::adjustColumnsWidth()
	{
		if (m_table == nullptr)
		{
			return;
		}

		double totalWidth = m_table->viewport()->size().width();

		for (int c = 0; c < m_table->horizontalHeader()->count(); c++)
		{
			int columnWidth = static_cast<int>(totalWidth * m_model.columnWidthPercent(c));

			if (columnWidth >= totalWidth)
			{
				columnWidth = 100;
			}

			m_table->setColumnWidth(c, columnWidth);
		}
	}

	void LogFileDialog::searchIssue(bool forward)
	{
		// Turn off autoscroll
		//
		m_autoScroll->setChecked(false);

		// Find an index with issue
		//
		int row = -1;

		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == true)
		{
			row = forward ? 0 : m_model.rowCount() - 1;
		}
		else
		{
			row = selectedIndexes[0].row();
		}

		int issueRow = m_model.searchIssue(row, forward);
		if (issueRow != -1)
		{
			m_table->selectionModel()->select(m_model.index(issueRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

			m_table->setCurrentIndex(m_model.index(issueRow, 0));

			m_table->scrollTo(m_model.index(issueRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Issue was not found!"));
		}
	}


	void LogFileDialog::onTypeComboIndexChanged(int index)
	{
		Q_UNUSED(index);

		int filterRecordTypeMask = MessageType::All;

		if (m_recordTypeCombo != nullptr)
		{
			int typeComboIndex = m_recordTypeCombo->currentIndex();
			if (typeComboIndex < 0)
			{
				assert(false);
				return;
			}

			filterRecordTypeMask = m_recordTypeCombo->itemData(typeComboIndex, Qt::UserRole).toInt();
		}

		m_model.setFilterRecordTypeMask(filterRecordTypeMask);

		m_model.fillRecords();

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));
	}

	void LogFileDialog::onTimeFilter()
	{
		DialogTimeFilter d(m_model.filterTimeFrom(), m_model.filterTimeTo(), this);
		if (d.exec() == QDialog::Accepted)
		{
			qint64 filterTimeFrom = d.filterTimeFrom();
			qint64 filterTimeTo = d.filterTimeTo();

			if (filterTimeFrom != -1 && filterTimeTo != -1)
			{
				//m_timeFilterButton->setText(tr("Time Filter is on"));
				m_timeFilterButton->setStyleSheet("QPushButton { font-weight: bold; }");
			}
			else
			{
				//m_timeFilterButton->setText(tr("Time Filter is off"));
				m_timeFilterButton->setStyleSheet({});
			}

			m_model.setFilterTimeFrom(filterTimeFrom);
			m_model.setFilterTimeTo(filterTimeTo);

			m_model.fillRecords();

			m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));

			return;
		}
	}

	void LogFileDialog::onFilter()
	{
		m_model.setFilterText(m_filterLineEdit->text());

		m_model.fillRecords();

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));
	}

	void LogFileDialog::onFilterEdited(const QString& text)
	{
		m_filter->setEnabled(text.isEmpty() == false);
		m_search->setEnabled(text.isEmpty() == false);
	}

	void LogFileDialog::onSearch()
	{
		// Turn off autoscroll
		//
		m_autoScroll->setChecked(false);

		// Find an index with issue
		//
		int row = 0;

		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == false)
		{
			row = selectedIndexes[0].row();
		}

		std::string findText = m_filterLineEdit->text().toLocal8Bit().toStdString();

		if (findText.empty() == true)
		{
			return;
		}

		int foundRow = m_model.searchRecord(row, findText);
		if (foundRow != -1)
		{
			m_table->selectionModel()->select(m_model.index(foundRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

			m_table->setCurrentIndex(m_model.index(foundRow, 0));

			m_table->scrollTo(m_model.index(foundRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Text was not found!"));
		}
	}

	void LogFileDialog::onAllSessionsClicked()
	{
		m_worker->read(m_allSessions->isChecked() == false);

		enableControls(false);

		m_progressDialog.exec();
	}

	void LogFileDialog::onLoadComplete()
	{
		std::vector<LogFileRecord> loadResult;

		m_worker->getLoadedData(&loadResult);

		m_model.setRecords(&loadResult);

		m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_worker->getCurrentFileName()));

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));

		m_table->scrollToBottom();

		adjustColumnsWidth();

		enableControls(true);
	}

	void LogFileDialog::onLoadFromFileComplete(const QString& fileName)
	{
		std::vector<LogFileRecord> loadResult;

		m_worker->getLoadedData(&loadResult);

		m_model.setRecords(&loadResult);

		setWindowTitle(fileName);

		m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(fileName));

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));

		m_table->scrollToBottom();

		adjustColumnsWidth();

		enableControls(true);
	}

	void LogFileDialog::onRecordArrived(LogFileRecord record)
	{
		if (isVisible() == false)
		{
			return;
		}

		if (m_loadedFromFile == true)
		{
			return;
		}

		bool modelWasEmpty = m_model.recordsCount() == 0;

		m_model.addRecord(record);

		if (modelWasEmpty == true)
		{
			adjustColumnsWidth();
		}

		m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_worker->getCurrentFileName()));

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));

		if (m_autoScroll->isChecked() == true)
		{
			m_table->scrollToBottom();
		}
	}

	void LogFileDialog::onCellCopyKeyPressed()
	{
		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == true)
		{
			return;
		}

		std::sort(selectedIndexes.begin(), selectedIndexes.end(), [](const QModelIndex& left, const QModelIndex& right) -> bool
		{
			if (left.row() == right.row())
			{
				return left.column() < right.column();
			}

			return left.row() < right.row();
		}
		);

		// Calculate the leftmost column
		//
		int firstColumn = selectedIndexes[0].column();

		for (const QModelIndex& mi : selectedIndexes)
		{
			if (mi.column() < firstColumn)
			{
				firstColumn = mi.column();
			}
		}

		// Build the result
		//
		QString result;

		int lastRow = selectedIndexes[0].row();

		int lastColumn = firstColumn;

		for (const QModelIndex& mi : selectedIndexes)
		{
			if (lastRow != mi.row())
			{
				// Switch to the next row
				//
				lastRow = mi.row();
				lastColumn = firstColumn;

				result += "\n";
			}

			// Fill tabs between columns
			//
			for (int c = lastColumn; c < mi.column(); c++)
			{
				result += "\t";
			}

			lastColumn = mi.column();

			result += m_model.data(mi, Qt::DisplayRole).toString().trimmed();
		}

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->clear();
		clipboard->setText(result);

		return;
	}

	void LogFileDialog::turnOffAutoscroll()
	{
		if (m_autoScroll->isChecked() == true)
		{
			m_autoScroll->setChecked(false);
		}
	}

	void LogFileDialog::onLoad()
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), QString(), "Log Files (*.log)");
		if (fileName.isEmpty() == true)
		{
			return;
		}

		m_loadedFromFile = true;

		m_autoScroll->setChecked(false);

		enableControls(false);

		m_worker->readFromFile(QDir::toNativeSeparators(fileName));

		m_progressDialog.exec();

		return;
	}

	void LogFileDialog::onClear()
	{
		static std::vector<LogFileRecord> emptyRecords;
		m_model.setRecords(&emptyRecords);

		m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_model.rowCount()).arg(m_model.errorCount()).arg(m_model.warningCount()));
	}

	void LogFileDialog::onExport()
	{
		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save File"),
														"Untitled.log",
														tr("Log files (*.log)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}

		std::vector<LogFileRecord> exportRecords;

		m_model.getFilteredRecords(&exportRecords);
		if (exportRecords.empty() == true)
		{
			QMessageBox::warning(this, qAppName(), tr("No data to export!"));
			return;
		}

		QFile data(fileName);
		if (data.open(QFile::WriteOnly | QFile::Truncate) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("File creation error!"));
			return;
		}


		QTextStream out(&data);

		for (const LogFileRecord& rec : exportRecords)
		{
			out << rec.toString(m_worker->sessionHashString());
		}

		QMessageBox::information(this, qAppName(), tr("Export complete."));
		return;
	}

	void LogFileDialog::onPrevIssue()
	{
		searchIssue(false);
	}

	void LogFileDialog::onNextIssue()
	{
		searchIssue(true);
	}

	void LogFileDialog::onLinkActivated(const QString& link)
	{
#if defined(Q_OS_WIN)
		QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(link)});
#else
		QFileInfo info(link);
		QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir()? link : info.path()));
#endif
	}

	//
	// LogFile
	//
	LogFile::LogFile(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount, bool addAppInfoOnStart)
	{
		QUuid uuid = QUuid::createUuid();

		m_sessionHash = ::calcHash(uuid.toString());

		m_logFileWorker = new LogFileWorker(logName, QDir::toNativeSeparators(path), maxFileSize, maxFilesCount, m_sessionHash);

		connect(m_logFileWorker, &LogFileWorker::writeFailure, this, &LogFile::onFlushFailure);

		m_logThread.addWorker(m_logFileWorker);
		m_logThread.start();

		// Register LogFileRecord meta type
		//
		static int regLogFileRecordMetaType = false;

		if (regLogFileRecordMetaType == false)
		{
			regLogFileRecordMetaType = true;
			qRegisterMetaType<LogFileRecord>();
		}

		if (addAppInfoOnStart == true)
		{
			// --
			//
			LogFile::writeText("---");
			LogFile::writeMessage(tr("Logging started:"));
			LogFile::writeMessage(tr("-- SessionHash: %1").arg(m_sessionHash, 16, 16, QChar('0')));

			if (qApp != nullptr)
			{
				LogFile::writeMessage(QString("-- Application: %1 v%2").arg(qApp->applicationName(), qApp->applicationVersion()));
				LogFile::writeMessage(QString("-- Path: %1").arg(qApp->applicationFilePath()));
				LogFile::writeMessage(QString("-- CommandLine: %1").arg(qApp->arguments().join(" ")));
				LogFile::writeMessage(QString("-- ProcessID: %1 (0x%2)").arg(qApp->applicationPid()).arg(qApp->applicationPid(), 8, 16, QChar('0')));
			}

			QOperatingSystemVersion os = QOperatingSystemVersion::current();
			LogFile::writeMessage(QString("-- OS: %1 v%2.%3.%4").arg(os.name()).arg(os.majorVersion()).arg(os.minorVersion()).arg(os.microVersion()));

#ifdef Q_OS_WINDOWS
			char *usernameBuffer;
			size_t userNameLength;
			errno_t err = _dupenv_s(&usernameBuffer, &userNameLength, "USERNAME");
			QString userName = err == 0 ? usernameBuffer : nullptr;
#endif
#if defined(Q_OS_MACOS) or defined(Q_OS_LINUX)
			QString userName{getenv("USER")};			// For Mac or Linux
#endif
			LogFile::writeMessage(QString("-- User: %1").arg(userName));
		}

		return;
	}

	LogFile::~LogFile()
	{
		m_logFileWorker = nullptr;

		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}

		return;
	}

	bool LogFile::writeMessage(const QString& text)
	{
		return write(MessageType::Message, text);
	}

	bool LogFile::writeAlert(const QString& text)
	{
		m_alertAckCounter++;

		emit alertArrived(text);

		return write(MessageType::Alert, text);
	}

	bool LogFile::writeError(const QString& text)
	{
		m_errorAckCounter++;

		return write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text)
	{
		m_warningAckCounter++;

		return write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text)
	{
		return write(MessageType::Text, text);
	}

	bool LogFile::writeArray(const QStringList& textArray)
	{
		if (m_logFileWorker == nullptr)
		{
			return true;
		}

		return m_logFileWorker->writeArray(textArray);
	}

	bool LogFile::write(MessageType type, const QString& text)
	{
		if (m_logFileWorker == nullptr)
		{
			return true;
		}

		return m_logFileWorker->write(type, text);
	}

	void LogFile::view(QWidget* parent, bool showType, bool headerVisible, std::vector<std::pair<QString, double>> headerTitles)
	{
		m_alertAckCounter = 0;
		m_errorAckCounter = 0;
		m_warningAckCounter = 0;

		if (m_logDialog != nullptr)
		{
			if (m_logDialog->isActiveWindow() == false)
			{
				m_logDialog->activateWindow();
			}
		}
		else
		{
			m_logDialog = new LogFileDialog(m_logFileWorker, parent, showType, headerVisible, headerTitles);

			connect(m_logDialog, &QDialog::finished, this, &LogFile::onDialogFinished);

			m_logDialog->show();
		}

		UiTools::adjustDialogPlacement(m_logDialog);

		return;
	}

	int LogFile::alertAckCounter() const
	{
		return m_alertAckCounter;
	}

	int LogFile::errorAckCounter() const
	{
		return m_errorAckCounter;
	}

	int LogFile::warningAckCounter() const
	{
		return m_warningAckCounter;
	}

	QString LogFile::getCurrentFileName() const
	{
		return m_logFileWorker->getCurrentFileName();
	}

	QString LogFile::getLogPath() const
	{
		return m_logFileWorker->getLogPath();
	}


	void LogFile::onFlushFailure(QString errorString)
	{
		emit writeFailure(errorString);
	}

	void LogFile::onDialogFinished(int result)
	{
		Q_UNUSED(result);

		m_logDialog = nullptr;
	}

}

