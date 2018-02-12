#ifndef TUNINGLOG_H
#define TUNINGLOG_H

#include "../SimpleThread.h"
#include "../AppSignal.h"

#include <QTimer>
#include <QDateTime>
#include <QMutex>

namespace TuningLog
{
	struct TuningLogRecord
	{
		quint64 sessionHash;
		QDateTime time;
		QString equipmentId;
		QString userName;
		QString customAppSignalId;
		double oldValue = 0;
		double newValue = 0;
		int precision = 0;
		QString message;

		QString toString(const QString& sessionHashString);

	};

	class TuningLogWorker : public SimpleThreadWorker
	{
		Q_OBJECT
	public:

		TuningLogWorker(const QString& fileName, const QString& path, quint64 sessionHash);
		virtual ~TuningLogWorker();

		bool write(const AppSignalParam& asp, double oldValue, double newValue, const QString& userName);
		bool write(const QString& message, const QString& userName);

	protected:
		virtual void onThreadStarted();
		virtual void onThreadFinished();

	private:

		QString getLogFileName() const;

		bool writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount);

		bool flush();

	private slots:

		void slot_onTimer();

	signals:
		void flushFailure();

	private:

		QTimer *m_timer = nullptr;

		QMutex m_queueMutex;
		std::vector<TuningLogRecord> m_queue;

		QString m_logName;
		QString m_path;

		quint64 m_sessionHash = 0;
		QString m_sessionHashString;
	};

	class TuningLog : public QObject
	{
		Q_OBJECT
	public:

		TuningLog(const QString& logName, const QString& path = QString());
		virtual ~TuningLog();

		bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue, const QString& userName);
		bool write(const QString& message, const QString& userName);

	signals:
		void writeFailure();

	private slots:
		void onFlushFailure();

	private:

		TuningLogWorker* m_logFileWorker = nullptr;

		SimpleThread m_logThread;

		quint64 m_sessionHash;
	};
}

#endif // TUNINGLOG_H

