#pragma once


#include <QThread>
#include <QEventLoop>


class SimpleThreadWorker : public QObject
{
	Q_OBJECT

private slots:
	void slot_onThreadStarted();
	void slot_onThreadFinished();

private:
	virtual void onThreadStarted() {}
	virtual void onThreadFinished() {}

	friend class SimpleThread;
};


class SimpleThread : public QObject
{
	Q_OBJECT

protected:
	QThread m_thread;
	SimpleThreadWorker* m_worker = nullptr;

public:
	SimpleThread();
	SimpleThread(SimpleThreadWorker* worker);

	~SimpleThread();

	void setWorker(SimpleThreadWorker* worker);

	void start();
	void quit();
	void wait(unsigned long time = ULONG_MAX);

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
