#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <unordered_map>
#include <QMutex>

#include "../lib/Tuning/ITuningSignalManager.h"

struct TuningNewValue
{
	TuningValue value;
	bool isUnapplied = false;
};

class TuningSignalManager : public QObject, public ITuningSignalManager
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
	std::vector<AppSignalParam> signalList() const;
	std::vector<Hash> signalHashes() const;

	// Implementation ITuningSignalManager
	//
public:
	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;

	virtual AppSignalParam signalParam(Hash hash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual bool signalParam(Hash hash, AppSignalParam* result) const override;
	virtual bool signalParam(const QString& appSignalId, AppSignalParam* result) const override;

	virtual TuningSignalState state(Hash hash, bool* found) const override;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const override;

	// State manipulation
	//
public:
#ifdef Q_DEBUG
	void validateStates();
#endif
	void invalidateStates();

	void setState(const QString& appSignalId, const TuningSignalState& state);
	void setState(Hash signalHash, const TuningSignalState& state);
	void setState(const std::vector<TuningSignalState>& states);

	// Working with new values

	TuningValue newValue(Hash signalHash) const;
	void setNewValue(Hash signalHash, const TuningValue& value);

	bool newValueIsUnapplied(Hash signalHash) const;
	void setNewValueAsApplied(Hash signalHash);

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

	// New values storage
	//
	mutable QMutex m_newValuesMutex;						// For access to m_newValues
	std::unordered_map<Hash, TuningNewValue> m_newValues;
};


#endif // OBJECTMANAGER_H
