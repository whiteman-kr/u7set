#ifndef APPSIGNALMANAGER_H
#define APPSIGNALMANAGER_H

#include <map>
#include <unordered_map>
#include <QMutex>
#include "../lib/IAppSignalManager.h"


class AppSignalManager : public QObject, public IAppSignalManager
{
	Q_OBJECT

public:
	explicit AppSignalManager(QObject* parent = nullptr);
	virtual ~AppSignalManager();

public:
	void reset();

	// Signal Params
	//
	void addSignal(const AppSignalParam& appSignal);
	void addSignals(const std::vector<AppSignalParam>& appSignals);

	std::vector<AppSignalParam> signalList() const;
	std::vector<Hash> signalHashes() const;

	// Signal States
	//
	void invalidateSignalStates();

	void setState(const QString& appSignalId, const AppSignalState& state);
	void setState(Hash signalHash, const AppSignalState& state);
	void setState(const std::vector<AppSignalState>& states);

	// IAppSignalManager implememntation
	//
	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const override;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override;

signals:
	void addSignalToPriorityList(Hash signalHash) const;
	void addSignalsToPriorityList(QVector<Hash> signalHash) const;

private:
	mutable QMutex m_unitsMutex;
	std::map<int, QString> m_units;

	mutable QMutex m_paramsMutex;
	std::unordered_map<Hash, AppSignalParam> m_signalParams;

	mutable QMutex m_statesMutex;
	std::unordered_map<Hash, AppSignalState> m_signalStates;
};



#endif // APPSIGNALMANAGER_H
