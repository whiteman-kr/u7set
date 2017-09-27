#pragma once


#include <QThread>
#include <QEventLoop>


class SimpleThreadWorker : public QObject
{
	Q_OBJECT

private slots:
	void slot_onThreadStarted();
	void slot_onThreadFinished();

protected:
	virtual void onThreadStarted() {}
	virtual void onThreadFinished() {}

	bool quitRequested() const { return m_quitRequested; }

	void requestQuit() { m_quitRequested = true; }

private:
	bool m_quitRequested = false;

	friend class SimpleThread;
};


class SimpleThread : public QObject
{
	Q_OBJECT

protected:
	QThread m_thread;
	QList<SimpleThreadWorker*> m_workerList;

public:
	SimpleThread();
	SimpleThread(SimpleThreadWorker* worker);

	~SimpleThread();

	void addWorker(SimpleThreadWorker* worker);

	void start();
	void quit();
	bool wait(unsigned long time = ULONG_MAX);
	bool quitAndWait(unsigned long time = ULONG_MAX);

	bool isRunning() const;
	bool isFinished() const;

	virtual void beforeStart();
	virtual void beforeQuit();
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
