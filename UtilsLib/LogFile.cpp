#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "LogFile.h"
#include "./Ui/UiTools.h"

//#define LOGFILE_USE_HEADER	// Uncomment this to use header

namespace Log
{
	//
	// LogFileRecord
	//
	const std::array<QString, 6> messageTypeTextShort{"ERR", "WRN", "MSG", "ALERT", "TXT", "DATA"};
    const std::array<char, 6> messageTypeFirstLetter{'E', 'W', 'M', 'A', 'T', 'D'};

	const QString messageTimeFormat("dd.MM.yyyy hh:mm:ss.zzz");

	void replaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			 subject.replace(pos, search.length(), replace);
			 pos += replace.length();
		}
	}

	QString LogFileRecord::toString(const QString& sessionHashString) const
	{
		if (type == MessageType::Text)
		{
			return QString("%1\t%2\r\n")
					.arg(sessionHashString)
					.arg(QString::fromLocal8Bit(text.c_str()).replace('\n', "\\n"));
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
				.arg(QString::fromLocal8Bit(text.c_str()).replace('\n', "\\n"));
	}

    bool LogFileRecord::loadFromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash)
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

        // Custom time parsing from messageTimeFormat("dd.MM.yyyy hh:mm:ss.zzz"), because
        // time = QDateTime::fromString(str.c_str(), messageTimeFormat).toMSecsSinceEpoch() is very slow
        //
        do{
            time = 0;

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
            //timePtr += 4;

            time = QDateTime(QDate(yyyy, MM, dd), QTime(hh, mm, ss, zzz)).toMSecsSinceEpoch();
        }while(false);

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

            qint64 strLen = ptrEnd - ptr;
			if (strLen > MAX_STR_LEN)
			{
				Q_ASSERT(false);
				return false;
			}

			text.assign(ptr, strLen);
			replaceStringInPlace(text, "\\n", "\n");
			return true;
		}

        qint64 strLen = ptrEnd - ptr;
		if (strLen > MAX_STR_LEN)
		{
			Q_ASSERT(false);
			return false;
		}
		str.assign(ptr, strLen);

        if (*str.c_str() == messageTypeFirstLetter[Error])
		{
			type = MessageType::Error;
		}
		else
		{
            if (*str.c_str() == messageTypeFirstLetter[Warning])
            {
				type = MessageType::Warning;
			}
			else
			{
                if (*str.c_str() == messageTypeFirstLetter[Message])
                {
					type = MessageType::Message;
				}
				else
				{
                    if (*str.c_str() == messageTypeFirstLetter[Alert])
					{
						type = MessageType::Alert;
					}
					else
					{
                        if (*str.c_str() == messageTypeFirstLetter[Data])
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
		replaceStringInPlace(text, "\\n", "\n");
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

    bool LogFileWorker::loadInProgress() const
    {
        return m_loadInProgress;
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

		const QByteArray ba =  f.readAll();

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
				qint64 len = ptr - str + 1;	// Length of the string INCLUDING '\n'

                if (record.loadFromString(str, len, sessionHash) == true)
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
		std::shared_ptr<int> unlockerPtr(nullptr, [&](int*) { lockShared(false, nullptr); });

		// Read the log
		//
        m_loadInProgress = true;

        m_cancelLoad = false;

        for (int i = 0; i < m_maxFilesCount; i++)  // Read from older to newer file!
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
		std::shared_ptr<int> unlockerPtr(nullptr, [&](int*) { lockShared(false, nullptr); });

		// Read the log
		//

        m_loadInProgress = true;

        m_cancelLoad = false;

        readFileRecords(fileName, false/*currentSessionOnly*/);

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
    // LogRecordProxyModel
    //
    LogRecordProxyModel::LogRecordProxyModel(LogRecordModel* sourceModel):
        m_sourceModel(sourceModel)
    {
        connect(this, &QSortFilterProxyModel::modelAboutToBeReset, [this](){
            m_errorCount = 0;
            m_warningCount = 0;
        });

    }

    int LogRecordProxyModel::recordCount() const
    {
        return rowCount();
    }

    int LogRecordProxyModel::sourceRow(int row) const
    {
        return mapToSource(index(row, 0)).row();
    }

    int LogRecordProxyModel::errorCount() const
    {
        return m_errorCount;
    }

    int LogRecordProxyModel::warningCount() const
    {
        return m_warningCount;
    }


    int LogRecordProxyModel::filterRecordTypeMask() const
    {
        return m_filterRecordTypeMask;
    }

    void LogRecordProxyModel::setFilterRecordTypeMask(int value)
    {
        beginResetModel();

        m_filterRecordTypeMask = value;

        endResetModel();
    }

    QString LogRecordProxyModel::filterText() const
    {
        return QString::fromLocal8Bit(m_filterText.c_str());
    }

    void LogRecordProxyModel::setFilterText(const QString& value)
    {
        beginResetModel();

        m_filterText = value.toLocal8Bit().toStdString();

        endResetModel();
    }

    qint64 LogRecordProxyModel::filterTimeFrom() const
    {
        return m_filterTimeFrom;
    }

    qint64 LogRecordProxyModel::filterTimeTo() const
    {
        return m_filterTimeTo;
    }

    void LogRecordProxyModel::setFilterTime(qint64 from, quint64 to)
    {
        beginResetModel();

        m_filterTimeFrom = from;
        m_filterTimeTo = to;

        endResetModel();
    }

    bool LogRecordProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& /*sourceParent*/) const
    {
        const LogFileRecord& record = m_sourceModel->record(sourceRow);

        if (m_filterRecordTypeMask != MessageType::All)
        {
            if ((m_filterRecordTypeMask & (1 << record.type)) == 0)
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

        if (m_filterText.empty() == false)
        {
            if (record.text.find(m_filterText) == std::string::npos)
            {
                return false;
            }
        }

        if (record.type == MessageType::Error)
        {
            m_errorCount++;
        }

        if (record.type == MessageType::Warning)
        {
            m_warningCount++;
        }

        return true;
    }


	//
	// LogRecordModel
	//
    LogRecordModel::LogRecordModel(bool showTypeColumn, const QStringList &headerTitles):
		m_showTypeColumn(showTypeColumn)
	{
		int c = 0;

		m_columnsNames << tr("Time");
		m_columnTime = c++;

		if (m_showTypeColumn == true)
		{
			m_columnsNames << tr("Type");
			m_columnType = c++;
		}

        if (headerTitles.empty() == true)
		{
			m_columnsNames << tr("Message");
			m_columnText = c++;
		}
		else
		{
            for (const QString& s : headerTitles)
			{
				m_columnsNames << s;
			}

			m_columnText = c++;
		}
	}

	LogRecordModel::~LogRecordModel()
	{
    }

    void LogRecordModel::clear()
    {
        // Remove data from the model
        //
        if (rowCount() > 0)
        {
            beginResetModel();

            m_chunks.clear();

            endResetModel();
        }
    }

    void LogRecordModel::appendChunk(const std::shared_ptr<LogFileChunk>& chunk)
	{
        // Append data to the model
        //
        if (chunk->empty() == false)
        {
            int index = rowCount();

            int count = static_cast<int>(chunk->size());

            beginInsertRows(QModelIndex(), index, index + count - 1);

            m_chunks.insert(m_chunks.end(), chunk);

            endInsertRows();
       }
	}

    void LogRecordModel::appendRecord(const LogFileRecord& record)
	{
        if (m_chunks.empty() == true)
        {
            std::shared_ptr<LogFileChunk> chunk = std::make_shared<LogFileChunk>();
            m_chunks.insert(m_chunks.end(), chunk);
        }

        // Add record to last chunk
        //
        int index = rowCount();

        beginInsertRows(QModelIndex(), index, index);

        std::shared_ptr<LogFileChunk> ch = m_chunks[m_chunks.size() - 1];
        ch->push_back(record);

        endInsertRows();
	}

    const LogFileRecord& LogRecordModel::record(int row) const
    {
        int processedRows = 0;
        for (const auto& ch : m_chunks)
        {
            if (row >= processedRows && row < processedRows + static_cast<int>(ch->size()))
            {
                return (*ch)[row - processedRows];
            }
            processedRows += static_cast<int>(ch->size());
        }

        Q_ASSERT(false);
        static LogFileRecord emptyRecord;
        return emptyRecord;
    }

    QBrush LogRecordModel::color(const QModelIndex& index, bool selected) const
    {
		const LogFileRecord& rec = record(index.row());

		switch (rec.type)
		{
		case MessageType::Error:
		case MessageType::Alert:
			if (selected == true)
			{
				return QBrush{qRgb(0xFF, 0x00, 0x00)};
			}
			else
			{
				return QBrush{qRgb(0xE0, 0x33, 0x33)};
			}
		case MessageType::Warning:
			if (selected == true)
			{
				return QBrush{qRgb(0xD8, 0x52, 0x07)};
			}
			else
			{
				return QBrush{qRgb(0xF8, 0x72, 0x17)};
			}
		default:
			return {};
		}
    }

    int LogRecordModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);

        int result = 0;
        for (const auto& ch : m_chunks)
        {
            result += static_cast<int>(ch->size());
        }

        return result;
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
        const LogFileRecord& rec = record(index.row());

		if (role == Qt::DisplayRole)
		{
			int displayIndex = static_cast<int>(column);
			if (displayIndex == m_columnTime)
			{
                return QDateTime().fromMSecsSinceEpoch(rec.time).toString(messageTimeFormat);
			}

			if (displayIndex == m_columnType)
			{
                size_t intType = static_cast<size_t>(rec.type);
				if (intType >= messageTypeTextShort.size())
				{
					assert(false);
					return QString();
				}

				return messageTypeTextShort[intType];
			}

			if (displayIndex >= m_columnText)
			{
                if (rec.type == MessageType::Data)
				{
					int textColumnNo = displayIndex - m_columnText;

					if (textColumnNo < 0)
					{
						assert(false);
						return QVariant();
					}

                    QStringList textArray = QString::fromLocal8Bit(rec.text.c_str()).split('\t');

					if (textColumnNo < textArray.size())
					{
						return textArray[textColumnNo];
					}
				}
				else
				{
					return QString::fromLocal8Bit(rec.text.c_str()).replace('\n', ' ');
				}
			}
		}

		if (role == Qt::ToolTipRole)
		{
			// Display tooltip only for text if it contains '\n'
			if (static_cast<int>(column) >= m_columnText &&
					rec.type != MessageType::Data &&
					rec.text.find('\n') != std::string::npos)
			{
				return QString::fromLocal8Bit(rec.text.c_str());
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
    SelectionControlDelegate::SelectionControlDelegate(QObject* parent, LogRecordModel* model, LogRecordProxyModel* proxyModel) :
		QStyledItemDelegate(parent),
        m_model(model),
        m_proxyModel(proxyModel)
	{
	}

	void SelectionControlDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);
		
		bool active = option->state & QStyle::State_Active;
		bool selected = option->state & QStyle::State_Selected;

		QBrush br = m_model->color(m_proxyModel->mapToSource(index), selected);
		option->palette.setColor(QPalette::Text, br.color());
		
		// Set color for selected item (by default it is displayed by white)
		//
		if (selected == true)
		{
			if (br.style() == Qt::NoBrush && active == true)
			{
				// Use white color on selected items if control is active
				//
				option->palette.setColor(QPalette::HighlightedText, Qt::white);
			}
			else
			{
				option->palette.setColor(QPalette::HighlightedText, br.color());
			}
			if (active == true)
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0x90, 0xc8, 0xf6));
			}
			else
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0xe0, 0xe0, 0xe0));
			}
		}
		else
		{
			option->palette.setColor(QPalette::Base, Qt::white);
		}
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

		mainLayout->addWidget(new QLabel(tr("Start Time:")));
		m_timeFromEdit = new QDateTimeEdit(this);
		m_timeFromEdit->setDisplayFormat(messageTimeFormat);
		mainLayout->addWidget(m_timeFromEdit);

		mainLayout->addWidget(new QLabel(tr("End Time:")));
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

    LogFileDialog::LogFileDialog(LogFileWorker* worker, QWidget* parent, bool useMessageType, bool headerVisible, const QStringList &headerTitles) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
		m_worker(worker),
		m_model(useMessageType, headerTitles),
        m_proxyModel(&m_model)
	{
		setAttribute(Qt::WA_DeleteOnClose);

		setWindowTitle(tr("Log View - %1").arg(worker->logName()));

		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		QHBoxLayout* topLayout = new QHBoxLayout();
		mainLayout->addLayout(topLayout);

		if (useMessageType == true)
		{
			topLayout->addWidget(new QLabel(tr("Type:")));

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
        m_proxyModel.setSourceModel(&m_model);
        m_table->setModel(&m_proxyModel);

		m_table->verticalHeader()->hide();
        m_table->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
        m_table->setItemDelegate(new SelectionControlDelegate(this, &m_model, &m_proxyModel));

        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_table->setStyleSheet("QTableView::item { margin: 10px }");
        m_table->horizontalHeader()->setStretchLastSection(true);

        if (headerVisible == false)
		{
            m_table->horizontalHeader()->hide();
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
        connect(m_worker, &LogFileWorker::loadChunkComplete, this, &LogFileDialog::onLoadChunkComplete);
        connect(m_worker, &LogFileWorker::loadComplete, this, &LogFileDialog::onLoadComplete);
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

        m_worker->load(true/*currentSessionOnly*/);

		enableControls(false);
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

        if (m_worker->loadInProgress() == true)
        {
            m_worker->cancelLoad();
        }
	}

	void LogFileDialog::enableControls(bool enable)
	{
        m_allSessions->setEnabled(enable && m_loadedFromFile == false);

		m_clear->setEnabled(enable);
        m_load->setEnabled(enable);
		m_export->setEnabled(enable);
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
            row = forward ? 0 : m_proxyModel.rowCount() - 1;
		}
		else
		{
			row = selectedIndexes[0].row();
		}

        int issueRow = searchIssue(row, forward);
		if (issueRow != -1)
		{
            m_table->selectionModel()->select(m_proxyModel.index(issueRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

            m_table->setCurrentIndex(m_proxyModel.index(issueRow, 0));

            m_table->scrollTo(m_proxyModel.index(issueRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Issue was not found!"));
		}
	}

    int LogFileDialog::searchIssue(int startRow, bool forward) const
    {
        if (m_proxyModel.recordCount() == 0)
        {
            return -1;
        }

        int row = startRow;

        do
        {
            if (forward == true)
            {
                row++;
                if (row >= m_proxyModel.recordCount())	// Loop to start
                {
                    row = 0;
                }
            }
            else
            {
                row--;
                if (row < 0)	// Loop to end
                {
                    row = m_proxyModel.recordCount() - 1;
                }
            }

            int sourceRow =  m_proxyModel.sourceRow(row);

            const LogFileRecord& rec = m_model.record(sourceRow);

            if (rec.type == MessageType::Warning || rec.type == MessageType::Error)
            {
                return row;
            }
        }while (row != startRow);

        return -1;
    }

    int LogFileDialog::searchRecord(int startRow, const std::string& text) const
    {
        if (text.empty() == true)
        {
            return -1;
        }

        if (m_proxyModel.recordCount() == 0)
        {
            return -1;
        }

        int row = startRow;

        do
        {
            row++;
            if (row >= m_proxyModel.recordCount())	// Loop to start
            {
                row = 0;
            }

            int sourceRow =  m_proxyModel.sourceRow(row);

            const LogFileRecord& rec = m_model.record(sourceRow);

            if (rec.text.find(text) != std::string::npos)
            {
                return row;
            }
        }while (row != startRow);

        return -1;
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

        m_proxyModel.setFilterRecordTypeMask(filterRecordTypeMask);

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));
	}

	void LogFileDialog::onTimeFilter()
	{
        DialogTimeFilter d(m_proxyModel.filterTimeFrom(), m_proxyModel.filterTimeTo(), this);
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

            m_proxyModel.setFilterTime(filterTimeFrom, filterTimeTo);

            m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));

			return;
		}
	}

	void LogFileDialog::onFilter()
	{
        m_proxyModel.setFilterText(m_filterLineEdit->text());

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));
	}

	void LogFileDialog::onFilterEdited(const QString& text)
	{
		m_search->setEnabled(text.isEmpty() == false);
        m_filter->setEnabled(text.isEmpty() == false);

        if (m_proxyModel.filterText().isEmpty() == false && text.isEmpty() == true)
        {
            onFilter();
        }
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

        int foundRow = searchRecord(row, findText);
		if (foundRow != -1)
		{
            m_table->selectionModel()->select(m_proxyModel.index(foundRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

            m_table->setCurrentIndex(m_proxyModel.index(foundRow, 0));

            m_table->scrollTo(m_proxyModel.index(foundRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Text was not found!"));
		}
	}

	void LogFileDialog::onAllSessionsClicked()
	{
        if (m_loadedFromFile == true)
        {
            return;
        }

        m_model.clear();

        m_worker->load(m_allSessions->isChecked() == false);

		enableControls(false);
	}

    void LogFileDialog::onLoadChunkComplete()
    {
        bool modelWasEmpty = m_proxyModel.rowCount() == 0;

        std::vector<std::shared_ptr<LogFileChunk>> result;

        m_worker->getLoadedChunks(&result);

        if (result.empty() == true)
        {
            return;
        }

        for (const auto& chunk : result)
        {
            m_model.appendChunk(chunk);
        }

        if (modelWasEmpty == true)
        {
            for (int i = 0; i < m_table->horizontalHeader()->count() - 1; i++)
            {
                m_table->resizeColumnToContents(i);
            }
        }

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));

        if (m_autoScroll->isChecked() == true)
		{
			m_table->scrollToBottom();
        }
	}

    void LogFileDialog::onLoadComplete()
	{
        for (const auto& rec : m_arrivedRecords)
        {
            m_model.appendRecord(rec);
        }
        m_arrivedRecords.clear();

        //

        if (m_loadedFromFile == false)
        {
            m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_worker->getCurrentFileName()));
        }
        else
        {
            setWindowTitle(m_loadedFileName);
            m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_loadedFileName));
        }

        enableControls(true);

		m_table->scrollToBottom();
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

        if (m_worker->loadInProgress() == true)
        {
            // Log loading is in progress, write the record to an array and add later
            //
            m_arrivedRecords.push_back(record);
            return;
        }

        bool modelWasEmpty = m_proxyModel.rowCount() == 0;

        m_model.appendRecord(record);

        if (modelWasEmpty == true)
		{
            for (int i = 0; i < m_table->horizontalHeader()->count() - 1; i++)
            {
                m_table->resizeColumnToContents(i);
            }
        }

		m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_worker->getCurrentFileName()));

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));

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

            result += m_proxyModel.data(mi, Qt::DisplayRole).toString().trimmed();
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
		static QString path{"."};
		QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), path, "Log Files (*.log)");
		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		m_loadedFromFile = true;
        m_loadedFileName = fileName;

		m_autoScroll->setChecked(false);

        m_model.clear();

        m_worker->loadFromFile(QDir::toNativeSeparators(fileName));

        enableControls(false);

		return;
	}

	void LogFileDialog::onClear()
	{
        m_model.clear();

        m_allSessions->setChecked(false);

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel.rowCount()).arg(m_proxyModel.errorCount()).arg(m_proxyModel.warningCount()));

        if (m_loadedFromFile == true)
        {
            m_loadedFromFile = false;

            m_allSessions->setEnabled(true);

            setWindowTitle(tr("Log View - %1").arg(m_worker->logName()));
        }
    }

	void LogFileDialog::onExport()
	{
		static QString path{"."};
        QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save File"),
														path + QDir::separator() + "Untitled.log",
														tr("Log files (*.log)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		std::vector<LogFileRecord> exportRecords;

        int count = m_proxyModel.recordCount();
        exportRecords.reserve(count);

        for (int i = 0; i < count; i++)
        {
            int sourceRow = m_proxyModel.sourceRow(i);
            const LogFileRecord& rec = m_model.record(sourceRow);
            exportRecords.push_back(rec);
        }

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

	bool LogFile::writeMessage(const QString& text, const QString& /*tag*/)
	{
		return write(MessageType::Message, text);
	}

	bool LogFile::writeAlert(const QString& text, const QString& /*tag*/)
	{
		m_alertAckCounter++;

		emit alertArrived(text);

		return write(MessageType::Alert, text);
	}

	bool LogFile::writeError(const QString& text, const QString& /*tag*/)
	{
		m_errorAckCounter++;

		return write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text, const QString& /*tag*/)
	{
		m_warningAckCounter++;

		return write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text, const QString& /*tag*/)
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

    void LogFile::view(QWidget* parent, bool showType, bool headerVisible, const QStringList& headerTitles)
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

