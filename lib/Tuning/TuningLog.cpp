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
		if (message.isEmpty() == true)
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
		else
		{
			return QString("%1\t%2\t\t%3\t%4\r\n")
				.arg(sessionHashString)
				.arg(time.toString(messageTimeFormat))
				.arg(userName)
				.arg(message);
		}
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

	bool TuningLogWorker::write(const AppSignalParam& asp, double oldValue, double newValue, const QString& userName)
	{
		QMutexLocker l(&m_queueMutex);

		TuningLogRecord r;

		r.sessionHash = m_sessionHash;
		r.time = QDateTime::currentDateTime();
		r.userName = userName;
		r.equipmentId = asp.equipmentId();
		r.customAppSignalId = asp.customSignalId();
		r.oldValue = oldValue;
		r.newValue = newValue;
		r.precision = asp.precision();

		if (r.userName.isEmpty() == true)
		{
			r.userName = tr("Unknown");
		}

		m_queue.push_back(r);

		return true;
	}

	bool TuningLogWorker::write(const QString& message, const QString& userName)
	{
		QMutexLocker l(&m_queueMutex);

		TuningLogRecord r;

		r.sessionHash = m_sessionHash;
		r.time = QDateTime::currentDateTime();
		r.userName = userName;
		r.message = message;

		if (r.userName.isEmpty() == true)
		{
			r.userName = tr("Unknown");
		}

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

	QString TuningLogWorker::getLogFileName() const
	{
		QDate tm = QDate::currentDate();

		return QString("%1%2%3Signals_%4%5%6.log")
				.arg(m_path)
				.arg(QDir::separator())
				.arg(m_logName)
				.arg(QString::number(tm.year()).rightJustified(4, '0'))
				.arg(QString::number(tm.month()).rightJustified(2, '0'))
				.arg(QString::number(tm.day()).rightJustified(2, '0'));
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

	bool TuningLog::write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue, const QString& userName)
	{
		return m_logFileWorker->write(asp, oldValue.toDouble(), newValue.toDouble(), userName);
	}

	bool TuningLog::write(const QString& message, const QString& userName)
	{
		return m_logFileWorker->write(message, userName);
	}


	void TuningLog::onFlushFailure()
	{
		qDebug() << "Log write failure!";
		assert(false);
		emit writeFailure();
	}

}

