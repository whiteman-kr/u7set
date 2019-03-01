#ifndef WORKERBASE_H
#define WORKERBASE_H

#include <QObject>
#include <QVector>

#include "SerialPortWorker.h"

// ==============================================================================================

class WorkerBase : public QObject
{
	Q_OBJECT

public:
	explicit WorkerBase(QObject *parent = nullptr);
	~WorkerBase();

private:

	mutable QMutex				m_mutex;
	QVector<SerialPortWorker*>	m_workerList;

public:

	int							count() const;
	void						clear();

	int							append(SerialPortWorker* pWorker);
	SerialPortWorker*			at(int index) const;

signals:

public slots:
};

// ==============================================================================================

#endif // WORKERBASE_H
