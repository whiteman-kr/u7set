#ifndef APPSIGNALMANAGER_H
#define APPSIGNALMANAGER_H

#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include <QMutex>
#include "../lib/IAppSignalManager.h"
#include "../lib/ComparatorSet.h"



class AppSignalManager : public QObject, public IAppSignalManager
{
	Q_OBJECT

public:
	explicit AppSignalManager(QObject* parent = nullptr);
	virtual ~AppSignalManager() = default;

public:
	void reset();

	// Signal Params
	//
	void addSignal(const AppSignalParam& appSignal);
	void addSignals(const std::vector<AppSignalParam>& appSignals);

	std::vector<Hash> signalHashes() const;

	// Signal States
	//
	void invalidateSignalStates();

	void setState(const QString& appSignalId, const AppSignalState& state);
	void setState(Hash signalHash, const AppSignalState& state);
	void setState(const std::vector<AppSignalState>& states);

	// Setpoints/Comparators
	//
	void setSetpoints(ComparatorSet&& setpoints);
	void setSetpoints(const ComparatorSet& setpoints);

	// IAppSignalManager implememntation - AppSignals
	//
	virtual std::vector<AppSignalParam> signalList() const override;

	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const override;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override;

	virtual QStringList signalTags(Hash signalHash) const override;
	virtual QStringList signalTags(const QString& appSignalId) const override;

	virtual bool signalHasTag(Hash signalHash, const QString& tag) const override;
	virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override;

	// IAppSignalManager implememntation - Setpoints
	//
	virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override;

	// Extension
	//
public:
	AppSignalParam signalParamByEquipemntId(const QString& equipmentId, bool* found) const;

signals:
	void addSignalToPriorityList(Hash signalHash) const;
	void addSignalsToPriorityList(QVector<Hash> signalHash) const;

private:
	mutable QReadWriteLock m_paramsLocker;
	std::unordered_map<Hash, AppSignalParam> m_signalParams;		// Key is hash from AppSignalID (hash from hash here, not nice)
	std::unordered_map<QString, Hash> m_signalParamByEquipmentId;	// Key is EquipmentId - value is hash from AppSignalID

	mutable QReadWriteLock m_statesLocker;
	std::unordered_map<Hash, AppSignalState> m_signalStates;

	// ComparatorSet is threadsafe itself
	//
	ComparatorSet m_setpoints;
};



#endif // APPSIGNALMANAGER_H
