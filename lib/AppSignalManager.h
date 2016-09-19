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

	// Units
	//
	struct AppSignalUnits
	{
		int id;
		QString unit;
	};

	void setUnits(const std::vector<AppSignalUnits>& units);
	std::map<int, QString> units() const;
	QString units(int id) const;

	// Signal Params
	//
	void addSignal(const Signal& signal);

	std::vector<Signal> signalList() const;

	std::vector<Hash> signalHashes() const;

	bool signal(const QString& appSignalId, Signal* out) const;
	bool signal(Hash signalHash, Signal* out) const;

	Signal signal(const QString& appSignalId, bool* found) const;
	Signal signal(Hash signalHash, bool* found) const;

	// Signal States
	//
	void setState(const QString& appSignalId, const AppSignalState& state);
	void setState(Hash signalHash, const AppSignalState& state);

	AppSignalState signalState(Hash signalHash, bool* found = nullptr);
	AppSignalState signalState(const QString& appSignalId, bool* found = nullptr);

	int signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result);
	int signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result);


private:
	mutable QMutex m_unitsMutex;
	std::map<int, QString> m_units;

	mutable QMutex m_paramsMutex;
	std::unordered_map<Hash, Signal> m_signals;

	mutable QMutex m_statesMutex;
	std::unordered_map<Hash, AppSignalState> m_states;

};

#endif // SIGNALSET_H
