#ifndef SIGNALHISTORY_H
#define SIGNALHISTORY_H

#include "SignalBase.h"

// ==============================================================================================

class SignalForLog
{

public:

	SignalForLog();
	SignalForLog(const SignalForLog& from);
	SignalForLog(PS::Signal* m_pSignal, double prevState, double state);
	virtual ~SignalForLog();

public:

	void clear();

	QString time() const { return m_time; }
	void setTime(const QString& time) { m_time = time; }

	PS::Signal* signalPtr() const { return m_pSignal; }
	void setSignalPtr(PS::Signal* pSignal) { m_pSignal = pSignal; }

	double prevState() const { return m_prevState; }
	void setPrevState(double state) { m_prevState = state; }

	double state() const { return m_state; }
	void setState(double state) { m_state = state; }

	QString stateStr(double state) const;

	SignalForLog& operator=(const SignalForLog& from);

private:

	mutable QMutex m_signalMutex;

	QString m_time;

	PS::Signal* m_pSignal = nullptr;

	double m_prevState = 0;
	double m_state = 0;
};


// ==============================================================================================

class SignalHistory : public QObject
{
	Q_OBJECT

public:

	explicit SignalHistory(QObject *parent = nullptr);
	virtual ~SignalHistory();

public:

	void clear();
	int count() const;

	int append(const SignalForLog& signalLog);

	SignalForLog* signalPtr(int index) const;
	SignalForLog signal(int index) const;

	SignalHistory& operator=(const SignalHistory& from);

private:

	mutable QMutex m_signalMutex;
	QVector<SignalForLog> m_signalList;

signals:

	void signalCountChanged();
};

// ==============================================================================================

#endif // SIGNALHISTORY_H
