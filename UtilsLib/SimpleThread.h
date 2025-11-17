#pragma once

#include <set>
#include <atomic>

#include <QEventLoop>
#include <QThread>

class SimpleThread;

class SimpleThreadWorker : public QObject
{
	Q_OBJECT

public:
	SimpleThreadWorker(const QString& workerName = QString());
	virtual ~SimpleThreadWorker();

	QString workerName() const;

	void enableLog(bool enable);

protected:
	virtual void onThreadStarted();
	virtual void onThreadFinished();

	void printFunction(const QString& func);

private slots:
	void slot_onThreadStarted();
	void slot_onThreadFinished();

private:
	void setThread(SimpleThread* thread);
	void log(const QString& str);

private:
	QString m_workerName;
	bool m_enableLog = false;

	SimpleThread* m_thread = nullptr;
	std::atomic_bool m_finished = false;

	friend class SimpleThread;
};

#define PRINT_FUNC printFunction(__func__);

class SimpleThread : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY_MOVE(SimpleThread)

	static const unsigned long START_TIME_MS = 5000;
	static const unsigned long STOP_TIME_MS = 5000;

public:
	SimpleThread();
	SimpleThread(SimpleThreadWorker* worker);
	virtual ~SimpleThread();

public:
	void addWorker(SimpleThreadWorker* worker);

	void setPriority(QThread::Priority priority);

	void start(unsigned long time = START_TIME_MS);
	bool quitAndWait(unsigned long time = STOP_TIME_MS);

	bool isRunning() const;
	bool isFinished() const;

	void enableLog(bool enable);

signals:
	void quitRequested();

private:
	void workerStarted(SimpleThreadWorker* worker);
	void workerFinished(SimpleThreadWorker* worker);

	void log(const QString& str);

protected:
	QString m_threadName;
	bool m_enableLog = false;

	QThread m_thread;

	std::mutex m_mutex;

	std::set<SimpleThreadWorker*> m_workers;
	std::set<SimpleThreadWorker*> m_runningWorkers;
	int m_finishedWorkersCount = 0;
	std::atomic_bool m_started = false;

	std::condition_variable m_condVar;

	friend class SimpleThreadWorker;
};

class RunOverrideThread : public QThread
{
public:
	bool isQuitRequested() const { return m_quitRequested.load(); }

	bool quitAndWait(unsigned long time = ULONG_MAX)
	{
		m_quitRequested.store(true);
		return wait(time);
	}

private:
	std::atomic<bool> m_quitRequested = { false };
};

class WaitForSignalHelper : public QObject
{
	Q_OBJECT

private:
	bool m_timeout = false;
	QEventLoop m_eventLoop;

private slots:
	void slot_timeout();

public:
	WaitForSignalHelper(const QObject* sender, const char* signal);

	bool wait(int milliseconds);			// return true if signal received before timeout
											// return false if timeout elapsed

	bool waitForever() { return wait(0); }
};

class ThreadWithQuit
{
public:
	void quit()
	{
		m_quitRequested = true;
		wakeupToQuit();
	}

	bool isQuitRequested() const { return m_quitRequested; }

	virtual void wakeupToQuit() {}

protected:
	std::atomic_bool m_quitRequested{false};
};
