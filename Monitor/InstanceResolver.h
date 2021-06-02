#pragma once
#include <QSharedMemory>

class InstanceResolver : public QObject
{
	Q_OBJECT

public:
	InstanceResolver() = default;
	virtual ~InstanceResolver() = default;

public:
	bool init(QString instanceId, bool singleInstance);		// if returns false, then this instance must be closed
	bool reinit(QString instanceId, bool singleInstance);

protected:
	virtual void timerEvent(QTimerEvent* event) override;

signals:
	void activate();

private:
	enum class State
	{
		Idle,					// Instance created
		ActivationRequested,	// Activation is requested, App after activation must set state Idle
	};

	struct Data
	{
		State state = State::Idle;
	};

	std::unique_ptr<QSharedMemory> m_sm;
	int m_timerId = 0;
};

/*
Windows:
	QSharedMemory does not "own" the shared memory segment. When all threads or processes that have an instance of
	QSharedMemory attached to a particular shared memory segment have either destroyed their instance of QSharedMemory
	or exited, the Windows kernel releases the shared memory segment automatically.

Unix:
	QSharedMemory "owns" the shared memory segment. When the last thread or process that has an instance of QSharedMemory
	attached to a particular shared memory segment detaches from the segment by destroying its instance of QSharedMemory,
	the Unix kernel release the shared memory segment. But if that last thread or process crashes without running
	the QSharedMemory destructor, the shared memory segment survives the crash.
*/

