#ifndef SIGNALSET_H
#define SIGNALSET_H

#include <unordered_map>
#include <QMutex>
#include "../lib/Hash.h"
#include "../lib/AppSignal.h"

typedef quint64 SignalHash;

class AppSignalManager;


class ScriptSignalManager : public QObject
{
	Q_OBJECT

public:
	 explicit ScriptSignalManager(const AppSignalManager* signalManager);

	// Script Interface
	//
public slots:
	QVariant signalParam(QString signalId) const;		// Returns AppSignalParam
	QVariant signalParam(Hash signalHash) const;		// Returns AppSignalParam

	QVariant signalState(QString signalId) const;		// Returns AppSignalState
	QVariant signalState(Hash signalHash) const;		// Returns AppSignalState

private:
	 const AppSignalManager* m_signalManager = nullptr;
};


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
	void addSignal(const AppSignalParam& signal);

	std::vector<AppSignalParam> signalList() const;
	std::vector<Hash> signalHashes() const;

	AppSignalParam signalParam(const QString& appSignalId, bool* found) const;
	AppSignalParam signalParam(Hash signalHash, bool* found) const;

	// Signal States
	//
	void invalidateAllSignalStates();

	void setState(const QString& appSignalId, const AppSignalState& state);
	void setState(Hash signalHash, const AppSignalState& state);

	AppSignalState signalState(Hash signalHash, bool* found) const;
	AppSignalState signalState(const QString& appSignalId, bool* found) const;

	void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const;
	void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const;

private:
	mutable QMutex m_unitsMutex;
	std::map<int, QString> m_units;

	mutable QMutex m_paramsMutex;
	std::unordered_map<Hash, AppSignalParam> m_signalParams;

	mutable QMutex m_statesMutex;
	std::unordered_map<Hash, AppSignalState> m_signalStates;
};



#endif // SIGNALSET_H
