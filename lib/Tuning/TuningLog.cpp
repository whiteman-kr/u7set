#include "TuningLog.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QUuid>
#include "../Hash.h"

namespace TuningLog
{
	//
	// TuningLogRecord
	//

	const char* messageTimeFormat = {"dd.MM.yyyy hh:mm:ss.zzz"};

	QString TuningLogRecord::toString(const QString& sessionHashString)
	{
		return QString("%1\t%2\t\t%3\t%4\t%5\t%6 -> %7\r\n")
				.arg(sessionHashString)
				.arg(time.toString(messageTimeFormat))
				.arg(equipmentId)
				.arg(userName)
				.arg(customAppSignalId)
				.arg(QString::number(oldValue, 'f', precision))
				.arg(QString::number(newValue, 'f', precision));
	}

	//
	// TuningLogWorker
	//

	TuningLogWorker::TuningLogWorker(const QString& logName, const QString& path, quint64 sessionHash)
		:m_logName(logName),
		  m_path(path),
		  m_sessionHash(sessionHash),
		  m_sessionHashString(QString::number(sessionHash).leftJustified(21, ' '))
	{
		if (m_path.isEmpty() == true)
		{
			QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			m_path = QDir::toNativeSeparators(localAppDataPath);
		}
	}

	TuningLogWorker::~TuningLogWorker()
	{
	}

	bool TuningLogWorker::write(const AppSignalParam& asp, double oldValue, double newValue)
	{
		QMutexLocker l(&m_queueMutex);

		TuningLogRecord r;

		r.sessionHash = m_sessionHash;
		r.time = QDateTime::currentDateTime();
		r.userName = getCurrentUserName();
		r.equipmentId = asp.equipmentId();
		r.customAppSignalId = asp.customSignalId();
		r.oldValue = oldValue;
		r.newValue = newValue;
		r.precision = asp.precision();

		m_queue.push_back(r);

		return true;
	}

	bool TuningLogWorker::write(const QString& equipmentId, const QString& caption, double oldValue, double newValue)
	{
		QMutexLocker l(&m_queueMutex);

		TuningLogRecord r;

		r.sessionHash = m_sessionHash;
		r.time = QDateTime::currentDateTime();
		r.userName = getCurrentUserName();
		r.equipmentId = equipmentId;
		r.customAppSignalId = caption;
		r.oldValue = oldValue;
		r.newValue = newValue;
		r.precision = 0;

		m_queue.push_back(r);

		return true;
	}

	void TuningLogWorker::onThreadStarted()
	{
		// Start timer

		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &TuningLogWorker::slot_onTimer);
		m_timer->start(500);

	}

	void TuningLogWorker::onThreadFinished()
	{
		m_timer->stop();

		if (flush() == false)
		{
			emit flushFailure();
		}
	}

	QString TuningLogWorker::getCurrentUserName() const
	{
		QString userName = qgetenv("USER");									// get the user name in Linux

		if(userName.isEmpty())
		{
			userName = qgetenv("USERNAME"); // get the name in Windows
		}

		if(userName.isEmpty())
		{
			userName = "Unknown user";
		}

		return userName;
	}


	QString TuningLogWorker::getLogFileName() const
	{
		QDate tm = QDate::currentDate();

		return QString("%1%2%3_%4%5%6.log")
				.arg(m_path)
				.arg(QDir::separator())
				.arg(m_logName)
				.arg(QString::number(tm.year()))
				.arg(QString::number(tm.month()))
				.arg(QString::number(tm.day()));
	}

	bool TuningLogWorker::flush()
	{
		QMutexLocker l(&m_queueMutex);

		if (m_queue.size() == 0)
		{
			return true;
		}

		QString fileName = getLogFileName();

		// Open file for writing

		QFile file(fileName);

		if (file.open(QIODevice::Append | QIODevice::Text ) == false)
		{
			return false;
		}

		// Write records

		for (TuningLogRecord& record : m_queue)
		{
			file.write(record.toString(m_sessionHashString).toUtf8());
		}

		file.close();

		m_queue.clear();

		return true;
	}

	void TuningLogWorker::slot_onTimer()
	{
		if (flush() == false)
		{
			emit flushFailure();
		}
	}

	//
	// TuningLog
	//

	TuningLog::TuningLog(const QString& logName, const QString& path)
	{
		QUuid uuid = QUuid::createUuid();

		m_sessionHash = ::calcHash(uuid.toString());

		m_logFileWorker = new TuningLogWorker(logName, path, m_sessionHash);

		connect(m_logFileWorker, &TuningLogWorker::flushFailure, this, &TuningLog::onFlushFailure);

		m_logThread.addWorker(m_logFileWorker);
		m_logThread.start();
	}

	TuningLog::~TuningLog()
	{
		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}
	}

	bool TuningLog::write(const AppSignalParam& asp, double oldValue, double newValue)
	{
		return m_logFileWorker->write(asp, oldValue, newValue);
	}

	bool TuningLog::write(const QString& equipmentId, const QString& caption, double oldValue, double newValue)
	{
		return m_logFileWorker->write(equipmentId, caption, oldValue, newValue);
	}


	void TuningLog::onFlushFailure()
	{
		qDebug() << "Log write failure!";
		assert(false);
		emit writeFailure();
	}

}

