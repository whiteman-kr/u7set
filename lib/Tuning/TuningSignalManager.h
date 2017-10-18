#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <unordered_map>
#include <QMutex>

#include "../lib/Tuning/TuningSignalState.h"
#include "../AppSignal.h"

class TuningSignalManager : public QObject
{
	Q_OBJECT

public:
	TuningSignalManager();
	virtual ~TuningSignalManager();

public:
	void reset();

	bool load(const QByteArray& data);
	bool load(const ::Proto::AppSignalSet& message);

	// AppSignalParams
	//
	int signalsCount() const;
	bool signalExists(Hash hash) const;

	std::vector<AppSignalParam> signalList() const;
	std::vector<Hash> signalHashes() const;

	AppSignalParam signalParam(Hash hash, bool* found) const;
	AppSignalParam signalParam(const QString& appSignalId, bool* found) const;

	bool signalParam(Hash hash, AppSignalParam* result) const;
	bool signalParam(const QString& appSignalId, AppSignalParam* result) const;

	// States
	//
	void invalidateStates();

	TuningSignalState state(Hash hash, bool* found) const;
	TuningSignalState state(const QString& appSignalId, bool* found) const;

	void setState(const QString& appSignalId, const TuningSignalState& state);
	void setState(Hash signalHash, const TuningSignalState& state);
	void setState(const std::vector<TuningSignalState>& states);

	// Signals
	//
signals:
	void signalsLoaded();			// Emited when new signals loaded

	// Data
	//
private:

	// Objects storage
	//
	mutable QMutex m_signalsMutex;						// For access to m_signals
	std::unordered_map<Hash, AppSignalParam> m_signals;

	// States storage
	//
	mutable QMutex m_statesMutex;						// For access to m_states
	std::unordered_map<Hash, TuningSignalState> m_states;
};


#endif // OBJECTMANAGER_H
