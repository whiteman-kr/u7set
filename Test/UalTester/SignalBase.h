#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QVector>

#include "../../lib/Hash.h"
#include "../../lib/Signal.h"
#include "../../lib/AppSignal.h"

// ==============================================================================================

class TestSignal
{
public:

	TestSignal() {}
	explicit TestSignal(const Signal& signal)	{ setParam(signal); }
	virtual ~TestSignal() {}

private:

	Signal m_param;
	AppSignalState m_state;

public:

	Signal& param() { return m_param; }
	void setParam(const Signal& param) { m_param = param; }

	AppSignalState& state() { return m_state; }
	void setState(const AppSignalState& state) { m_state = state; }
};

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject *parent = nullptr);
	virtual ~SignalBase() {}

private:

	// all signals that received form CgfSrv
	//
	mutable QMutex			m_signalMutex;
	QMap<Hash, int>			m_signalHashMap;
	QVector<TestSignal>		m_signalList;

	// list of hashes in order to receive signal state form AppDataSrv
	//
	mutable QMutex			m_stateMutex;
	QVector<Hash>			m_requestStateList;

public:

	void					clear();

	// Signals
	//
	int						signalCount() const;
	void					clearSignalList();

	int						appendSignal(const Signal& param);

	TestSignal*				signalPtr(const QString& appSignalID);
	TestSignal*				signalPtr(const Hash& hash);
	TestSignal*				signalPtr(int index);

	TestSignal				signal(const QString& appSignalID);
	TestSignal				signal(const Hash& hash);
	TestSignal				signal(int index);

	Signal					signalParam(const QString& appSignalID);
	Signal					signalParam(const Hash& hash);
	Signal					signalParam(int index);

	void					setSignalParam(const QString& appSignalID, const Signal& param);
	void					setSignalParam(const Hash& hash, const Signal& param);
	void					setSignalParam(int index, const Signal& param);

	AppSignalState			signalState(const QString& appSignalID);
	AppSignalState			signalState(const Hash& hash);
	AppSignalState			signalState(int index);

	void					setSignalState(const QString& appSignalID, const AppSignalState& state);
	void					setSignalState(const Hash& hash, const AppSignalState& state);
	void					setSignalState(int index, const AppSignalState& state);

	// hashs for update signal state
	//
	void					clearHashForRequestState();
	int						hashForRequestStateCount() const;
	void					appendHashForRequestState(const QVector<Hash>& hashList);
	Hash					hashForRequestState(int index);

signals:

	void					updatedSignalParam(Hash signalHash);

public slots:

};

// ==============================================================================================

#endif // SIGNALBASE_H
