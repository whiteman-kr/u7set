#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "LogFile.h"

#include <queue>
#include <QSharedMemory>

#include "../UtilsLib/SimpleThread.h"

namespace Log
{
	//
	// LogFileRecord parsing
	//
	QString LogFileRecord::toString(const LogFileRecord& r, const QString& sessionHashString)
	{
		if (r.type == MessageType::Text)
		{
			return QString("%1\t%2\r\n").arg(sessionHashString).arg(QString::fromLocal8Bit(r.text.c_str()).replace('\n', "\\n"));
		}

		size_t intType = static_cast<int>(r.type);
		if (intType >= messageTypeTextShort.size())
		{
			assert(false);
			return QString();
		}

		return QString("%1\t%2\t\t%3\t%4\r\n")
			.arg(sessionHashString)
			.arg(QDateTime().fromMSecsSinceEpoch(r.time).toString(messageTimeFormat))
			.arg(messageTypeTextShort[intType])
			.arg(QString::fromLocal8Bit(r.text.c_str()).replace('\n', "\\n"));
	}

	bool LogFileRecord::fromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash, LogFileRecord& r)
	{
		const int MAX_STR_LEN = 1024;

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

		r.sessionHash = strtoull(ptr, nullptr, 16);

		if (currentSessionHash != 0 && r.sessionHash != currentSessionHash)
		{
			// Wrong session
			//
			return false;
		}

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;

		// Time
		//
		ptrEnd = static_cast<const char*>(memchr(ptr, '\t', bufLength));
		if (ptrEnd == nullptr)
		{
			// No time - do not read this line
			//
			return false;
		}

		// Custom time parsing from messageTimeFormat("dd.MM.yyyy hh:mm:ss.zzz"), because
		// time = QDateTime::fromString(str.c_str(), messageTimeFormat).toMSecsSinceEpoch() is very slow
		//
		do
		{
			r.time = 0;

			const char* timePtr = ptr;

			int dd = atoi(timePtr);

			if (*(timePtr + 2) != '.')
			{
				break;
			}
			timePtr += 3;

			int MM = atoi(timePtr);

			if (*(timePtr + 2) != '.')
			{
				break;
			}
			timePtr += 3;

			int yyyy = atoi(timePtr);

			if (*(timePtr + 4) != ' ')
			{
				break;
			}
			timePtr += 5;

			int hh = atoi(timePtr);

			if (*(timePtr + 2) != ':')
			{
				break;
			}
			timePtr += 3;

			int mm = atoi(timePtr);

			if (*(timePtr + 2) != ':')
			{
				break;
			}
			timePtr += 3;

			int ss = atoi(timePtr);

			if (*(timePtr + 2) != '.')
			{
				break;
			}
			timePtr += 3;

			int zzz = atoi(timePtr);

			if (*(timePtr + 3) != '\t')
			{
				break;
			}
			// timePtr += 4;

			r.time = QDateTime(QDate(yyyy, MM, dd), QTime(hh, mm, ss, zzz)).toMSecsSinceEpoch();
		} while (false);

		ptr = ptrEnd;
		ptr++;

		bufLength = bufEnd - ptr;

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

		// Type
		//
		r.type = MessageType::Text;

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

			qint64 strLen = ptrEnd - ptr;
			if (strLen > MAX_STR_LEN)
			{
				// This string is too long, cut it
				//
				strLen = MAX_STR_LEN;

				r.text.assign(ptr, strLen);

				for (int i = 0; i < 3; i++)
				{
					r.text[strLen - 3 + i] = '.';
				}
			}
			else
			{
				r.text.assign(ptr, strLen - 1 /*remove last \n*/);
			}
			replaceStringInPlace(r.text, "\\n", "\n");

			return true;
		}

		if (*ptr == messageTypeFirstLetter[Error])
		{
			r.type = MessageType::Error;
		}
		else
		{
			if (*ptr == messageTypeFirstLetter[Warning])
			{
				r.type = MessageType::Warning;
			}
			else
			{
				if (*ptr == messageTypeFirstLetter[Message])
				{
					r.type = MessageType::Message;
				}
				else
				{
					if (*ptr == messageTypeFirstLetter[Alert])
					{
						r.type = MessageType::Alert;
					}
					else
					{
						if (*ptr == messageTypeFirstLetter[Data])
						{
							r.type = MessageType::Data;
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

		qint64 strLen = ptrEnd - ptr;
		if (strLen > MAX_STR_LEN)
		{
			// This string is too long, cut it
			//
			strLen = MAX_STR_LEN;
			r.text.assign(ptr, strLen);

			for (int i = 0; i < 3; i++)
			{
				r.text[strLen - 3 + i] = '.';
			}
		}
		else
		{
			r.text.assign(ptr, strLen - 1 /*remove last \n*/);
		}
		replaceStringInPlace(r.text, "\\n", "\n");

		return true;
	}

	void LogFileRecord::replaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	} 
	
	//
	// LogFileWorker
	//
	class LogFileWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash);
		virtual ~LogFileWorker();

		// Writing functions
		//
		bool write(MessageType type, const QString& text);
		bool writeArray(const QStringList& textArray);

		// Loading funtcions
		//
		void load(bool currentSessionOnly);
		void loadFromFile(const QString& fileName);

		void cancelLoad();
		void getLoadedChunks(std::vector<std::shared_ptr<LogFileChunk>>* result);

		// Information functions
		//
		QString logName() const;

		QString getCurrentFileName() const;
		QString path() const;

		quint64 sessionHash() const;
		const QString& sessionHashString() const;

		bool loadInProgress() const;

		// Control functions
		//
		bool noDiskLog() const;
		void setNoDiskLog(bool value);

		std::vector<LogFileRecord> queue() const; // Get unwritten queue records, used in no-disk mode

	protected:
		virtual void onThreadStarted();
		virtual void onThreadFinished();

	private:
		QString getLogFileName(int index) const;

		bool readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount);
		bool writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount);
		bool lockShared(bool lock, bool* alreadyLocked = nullptr);
		bool flush(QString* errorString);
		bool switchToNextLogFile(QString* errorString);
		bool readFileRecords(const QString& fileName, bool currentSessionOnly);

	private slots:
		void slot_onTimer();
		void slot_load(bool currentSessionOnly);
		void slot_loadFromFile(QString fileName);

	signals:
		void writeFailure(QString errorString);
		void loadStart(bool currentSessionOnly);
		void loadLogFromFile(QString fileName);

		void loadChunkComplete();
		void loadComplete();
		void recordArrived(Log::LogFileRecord record);

	private:
		QTimer* m_timer = nullptr;

		mutable QMutex m_queueMutex;
		std::vector<LogFileRecord> m_queue;

		const QString m_logName;
		QString m_path;

		const int m_maxFileSize = 0;
		const int m_maxFilesCount = 0;
		int m_maxQueueSize = 1000;

		const quint64 m_sessionHash;
		const QString m_sessionHashString;

		const int m_serviceStringLength = 80;

		std::atomic<int> m_currentFileNumber = 0;

		std::atomic<bool> m_loadInProgress = false;
		std::atomic<bool> m_cancelLoad = false;
		std::atomic<bool> m_noDiskLog = false;

		QMutex m_loadLogMutex;                                    // Locks m_loadedChunks for reading data from log files
		std::queue<std::shared_ptr<LogFileChunk>> m_loadedChunks; // A queue contains chunks loaded from log files

		std::unique_ptr<QSharedMemory> m_sharedMemory;
	};
	
	//
	// LogFileWorker
	//
	LogFileWorker::LogFileWorker(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash) :
		m_logName(logName),
		m_path(path),
		m_maxFileSize(maxFileSize),
		m_maxFilesCount(maxFilesCount),
		m_sessionHash(sessionHash),
		m_sessionHashString(QString::number(sessionHash, 16).rightJustified(16, '0'))
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

				m_path.clear();
			}
		}

		m_maxQueueSize = std::clamp(QSettings().value("m_maxQueueSize", 1000).toInt(), 1000, 10000);
	}

	LogFileWorker::~LogFileWorker()
	{
		QSettings().setValue("m_maxQueueSize", m_maxQueueSize);
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

	void LogFileWorker::load(bool currentSessionOnly)
	{
		emit loadStart(currentSessionOnly);
	}

	void LogFileWorker::loadFromFile(const QString& fileName)
	{
		emit loadLogFromFile(fileName);
	}

	void LogFileWorker::cancelLoad()
	{
		m_cancelLoad = true;
	}

	void LogFileWorker::getLoadedChunks(std::vector<std::shared_ptr<LogFileChunk>>* result)
	{
		QMutexLocker l(&m_loadLogMutex);

		while (m_loadedChunks.empty() == false)
		{
			result->push_back(m_loadedChunks.front());
			m_loadedChunks.pop();
		}
	}

	QString LogFileWorker::logName() const
	{
		return m_logName;
	}

	QString LogFileWorker::getCurrentFileName() const
	{
		return getLogFileName(m_currentFileNumber);
	}

	QString LogFileWorker::path() const
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

	bool LogFileWorker::loadInProgress() const
	{
		return m_loadInProgress;
	}

	bool LogFileWorker::noDiskLog() const
	{
		return m_noDiskLog;
	}

	void LogFileWorker::setNoDiskLog(bool value)
	{
		m_noDiskLog = value;
	}

	std::vector<LogFileRecord> LogFileWorker::queue() const
	{
		QMutexLocker l(&m_queueMutex);
		return m_queue;
	}

	void LogFileWorker::onThreadStarted()
	{
		// Start timer
		//
		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &LogFileWorker::slot_onTimer);
		m_timer->start(500);

		// Get the number of last log file
		//
		QDir dir(path());

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

		connect(this, &LogFileWorker::loadStart, this, &LogFileWorker::slot_load);
		connect(this, &LogFileWorker::loadLogFromFile, this, &LogFileWorker::slot_loadFromFile);

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

		return QString("%1%2%3_%4.log").arg(path()).arg(QDir::separator()).arg(m_logName).arg(fileNumber);
	}

	bool LogFileWorker::readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount)
	{
		QFile file(fileName);

		if (file.exists() == false)
		{
			return false;
		}

		if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
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

		// Keep maximum queue size
		//
		m_queueMutex.lock();
		while (m_queue.size() > m_maxQueueSize)
		{
			m_queue.pop_back();
		}
		m_queueMutex.unlock();

		// In no-disk mode, just do nothing
		//
		if (m_noDiskLog == true)
		{
			return true;
		}

		// Write queue to the disk
		//
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
			return true;
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr,
										 [&](int*)
										 {
											 lockShared(false, nullptr);
										 });

		// Create the queue copy and empty it
		//
		m_queueMutex.lock();
		std::vector<LogFileRecord> queueCopy{std::move(m_queue)};
		m_queue = {};
		m_queue.reserve(64);
		m_queueMutex.unlock();
		if (queueCopy.empty() == true)
		{
			return true;
		}

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

		if (file.open(QIODevice::Append | QIODevice::Text) == false)
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
			if (file.write(LogFileRecord::toString(record, m_sessionHashString).toLocal8Bit()) == -1)
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

	bool LogFileWorker::readFileRecords(const QString& fileName, bool currentSessionOnly)
	{
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

		const QByteArray ba = f.readAll();

		const char* ptr = ba.data();

		const char* endPtr = ptr + ba.length();

		const quint64 sessionHash = currentSessionOnly == true ? m_sessionHash : 0;

		const int maxReadResultSize = 1000;

		std::vector<LogFileRecord> readResult;
		readResult.reserve(maxReadResultSize);

		while (ptr != nullptr && ptr != endPtr)
		{
			const char* str = ptr;

			ptr = static_cast<const char*>(memchr(ptr, '\n', endPtr - ptr));

			if (ptr != nullptr)
			{
				qint64 len = ptr - str + 1; // Length of the string INCLUDING '\n'

				if (LogFileRecord::fromString(str, len, sessionHash, record) == true)
				{
					readResult.push_back(record);

					// Emit a signal that chunk is ready when its size exceeds maxReadResultSize records
					//
					if (readResult.size() >= maxReadResultSize)
					{
						std::shared_ptr<LogFileChunk> chunk = std::make_shared<LogFileChunk>();
						*chunk = std::move(readResult);
						readResult = {};

						{
							QMutexLocker l(&m_loadLogMutex);
							m_loadedChunks.push(chunk);
						}

						emit loadChunkComplete();
					}
				}

				ptr++;
			}

			if (m_cancelLoad == true)
			{
				break;
			}
		}

		// Emit a signal that last chunk is ready
		//
		if (readResult.empty() == false)
		{
			std::shared_ptr<LogFileChunk> chunk = std::make_shared<LogFileChunk>();
			*chunk = std::move(readResult);
			readResult = {};

			{
				QMutexLocker l(&m_loadLogMutex);
				m_loadedChunks.push(chunk);
			}

			emit loadChunkComplete();
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

				emit loadComplete();
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
			emit loadComplete();
			return;
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr,
										 [&](int*)
										 {
											 lockShared(false, nullptr);
										 });

		// Read the log
		//
		m_loadInProgress = true;

		m_cancelLoad = false;

		for (int i = 0; i < m_maxFilesCount; i++) // Read from older to newer file!
		{
			readFileRecords(getLogFileName(i), currentSessionOnly);

			if (m_cancelLoad == true)
			{
				break;
			}
		}

		m_loadInProgress = false;

		emit loadComplete();
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

				emit loadComplete();
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
			emit loadComplete();
			return;
		}

		// Create unlocker pointer
		//
		std::shared_ptr<int> unlockerPtr(nullptr,
										 [&](int*)
										 {
											 lockShared(false, nullptr);
										 });

		// Read the log
		//

		m_loadInProgress = true;

		m_cancelLoad = false;

		readFileRecords(fileName, false /*currentSessionOnly*/);

		m_loadInProgress = false;

		emit loadComplete();
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
	// LogFile
	//
	LogFile::LogFile(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount, bool addAppInfoOnStart)
	{
		QUuid uuid = QUuid::createUuid();

		Hash sessionHash = ::calcHash(uuid.toString());

		m_worker = new LogFileWorker(logName, QDir::toNativeSeparators(path), maxFileSize, maxFilesCount, sessionHash);

		connect(m_worker, &LogFileWorker::loadChunkComplete, this, &LogFile::loadChunkComplete);
		connect(m_worker, &LogFileWorker::loadComplete, this, &LogFile::loadComplete);
		connect(m_worker, &LogFileWorker::recordArrived, this, &LogFile::recordArrived);
		connect(m_worker, &LogFileWorker::writeFailure, this, &LogFile::onFlushFailure);

		m_logThread.addWorker(m_worker);
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
			LogFile::writeMessage(tr("-- SessionHash: %1").arg(sessionHash, 16, 16, QChar('0')));

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
		m_worker = nullptr;

		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}

		return;
	}

	bool LogFile::writeMessage(const QString& text, const QString& /*tag*/)
	{
		return m_worker->write(MessageType::Message, text);
	}

	bool LogFile::writeAlert(const QString& text, const QString& /*tag*/)
	{
		m_alertAckCounter++;

		emit alertArrived(text);

		return m_worker->write(MessageType::Alert, text);
	}

	bool LogFile::writeError(const QString& text, const QString& /*tag*/)
	{
		m_errorAckCounter++;

		return m_worker->write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text, const QString& /*tag*/)
	{
		m_warningAckCounter++;

		return m_worker->write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text, const QString& /*tag*/)
	{
		return m_worker->write(MessageType::Text, text);
	}

	bool LogFile::writeArray(const QStringList& textArray)
	{
		if (m_worker == nullptr)
		{
			return true;
		}

		return m_worker->writeArray(textArray);
	}

    void LogFile::resetAckCounters()
	{
		m_alertAckCounter = 0;
		m_errorAckCounter = 0;
		m_warningAckCounter = 0;
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

	QString LogFile::logName() const
	{
		return m_worker->logName();
	}

	QString LogFile::getCurrentFileName() const
	{
		return m_worker->getCurrentFileName();
	}

	QString LogFile::getLogPath() const
	{
		return m_worker->path();
	}

	void LogFile::load(bool currentSessionOnly) 
	{
		return m_worker->load(currentSessionOnly);
	}
	
	void LogFile::loadFromFile(const QString& fileName)
	{
		return m_worker->loadFromFile(fileName);
	}

	void LogFile::cancelLoad()
	{
		m_worker->cancelLoad();
	}
	
	bool LogFile::loadInProgress() const
	{
		return m_worker->loadInProgress();
	}

	void LogFile::getLoadedChunks(std::vector<std::shared_ptr<LogFileChunk>>* result) 
	{
		return m_worker->getLoadedChunks(result);
	}

	quint64 LogFile::sessionHash() const
	{
		return m_worker->sessionHash();
	}

	QString LogFile::sessionHashString() const
	{
		return m_worker->sessionHashString();
	}
	
	bool LogFile::noDiskLog() const 
	{
		return m_worker->noDiskLog();
	}

	void LogFile::setNoDiskLog(bool value) 
	{
		if (m_worker == nullptr)
		{
			Q_ASSERT(m_worker);
			return;
		}
		m_worker->setNoDiskLog(value);
	}

	std::vector<LogFileRecord> LogFile::queue() const
	{
		return m_worker->queue();
	}

	void LogFile::onFlushFailure(QString errorString)
	{
		emit writeFailure(errorString);
	}
}

#include "LogFile.moc"