#ifndef SIGNALSET_H
#define SIGNALSET_H

#include <QMutex>
#include <unordered_map>
#include "../lib/Hash.h"
#include "../lib/AppSignal.h"

typedef quint64 SignalHash;

class QJSEngine;
class AppSignalManager;


class ScriptSignalManager : public QObject
{
	Q_OBJECT

public:
	 ScriptSignalManager(QJSEngine* engine, const AppSignalManager* signalManager);

	// Script Interface
	//
public slots:
	virtual QObject* signal(QString signalId) const;		// Returns Signal*
	virtual QObject* signal(Hash signalHash) const;			// Returns Signal*

	virtual QObject* signalState(QString signalId) const;	// Returns AppSignalState*
	virtual QObject* signalState(Hash signalHash) const;	// Returns AppSignalState*

private:
	 QJSEngine* m_engine = nullptr;
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

	bool signal(const QString& appSignalId, AppSignalParam* out) const;
	bool signal(Hash signalHash, AppSignalParam* out) const;

	AppSignalParam signal(const QString& appSignalId, bool* found) const;
	AppSignalParam signal(Hash signalHash, bool* found) const;

	// Signal States
	//
	void setState(const QString& appSignalId, const AppSignalState& state);
	void setState(Hash signalHash, const AppSignalState& state);

	AppSignalState signalState(Hash signalHash, bool* found) const;
	AppSignalState signalState(const QString& appSignalId, bool* found) const;

	int signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result) const;
	int signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result) const;

	// Script Interface
	//
//public:
	virtual QObject* signal(QString signalId) const;		// Returns AppSignalParam*
	virtual QObject* signal(Hash signalHash) const;			// Returns AppSignalParam*

//	virtual QObject* signalState(QString signalId) const override;	// Returns AppSignalState*
//	virtual QObject* signalState(Hash signalHash) const override;	// Returns AppSignalState*

private:
	mutable QMutex m_unitsMutex;
	std::map<int, QString> m_units;

	mutable QMutex m_paramsMutex;
	std::unordered_map<Hash, AppSignalParam> m_signals;

	mutable QMutex m_statesMutex;
	std::unordered_map<Hash, AppSignalState> m_states;
};



#endif // SIGNALSET_H
