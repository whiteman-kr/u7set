#ifndef SIGNALSET_H
#define SIGNALSET_H

#include <QMutex>
#include <unordered_map>
#include "../lib/Hash.h"
#include "../lib/Signal.h"

typedef quint64 SignalHash;

class AppSignalManager
{
public:
	AppSignalManager();
	virtual ~AppSignalManager();

public:
	void reset();

	void addSignal(const Signal& signal);

	bool signal(const QString& appSignalId, Signal* out);
	bool signal(Hash signalHash, Signal* out);

	void setState(Hash signalHash, const AppSignalState& state);
	AppSignalState signalState(Hash signalHash, bool* found = nullptr);
	AppSignalState signalState(const QString& appSignalId, bool* found = nullptr);

private:
	QMutex m_paramMutex;
	std::unordered_map<Hash, Signal> m_signals;

	QMutex m_stateMutex;
	std::unordered_map<Hash, AppSignalState> m_states;

};

#endif // SIGNALSET_H
