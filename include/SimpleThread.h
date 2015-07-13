#pragma once


#include <QThread>


class SimpleThreadWorker : public QObject
{
	Q_OBJECT

private slots:
	void slot_onThreadStarted();
	void slot_onThreadFinished();

public:
	virtual void onThreadStarted() {}
	virtual void onThreadFinished() {}

	friend class SimpleThread;
};


class SimpleThread : public QObject
{
	Q_OBJECT

private:
	QThread m_thread;
	SimpleThreadWorker* m_worker = nullptr;

public:
	SimpleThread();
	SimpleThread(SimpleThreadWorker* worker);

	~SimpleThread();

	void setWorker(SimpleThreadWorker* worker);

	void start();
	void quit();

	virtual void beforeStart();
	virtual void beforeQuit();
};
