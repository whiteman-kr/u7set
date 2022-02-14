#pragma once

#include <map>
#include <set>
#include <memory>
#include <vector>
#include <unordered_map>
#include <QReadWriteLock>
#include "../UtilsLib/ILogFile.h"
#include "../lib/IAppSignalManager.h"
#include "../lib/ComparatorSet.h"



class MonitorConfigController;

struct SourceState
{
	AppSignalState state{};
	Qt::HANDLE sourceThreadId{0};
	std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{};	// State last time received or updated
};


struct Sources
{
	size_t size = 0;
	std::array<SourceState, 4> sources{};		// 4 is maximum possible ways of getting signal (2 with regular channel, 2 with recent channel)

	void set(const AppSignalState& state, Qt::HANDLE sourceThreadId);
	void invalidateSource(Qt::HANDLE sourceThreadId);
	const AppSignalState& get() const;
};


class MonitorSignalManager final : public QObject, public IAppSignalManager
{
	Q_OBJECT

public:
	explicit MonitorSignalManager(MonitorConfigController& configController, ILogFile* logFile, QObject* parent = nullptr);
	virtual ~MonitorSignalManager() = default;

public:
	void reset();

	// Signal Params
	//
	void addSignal(const AppSignalParam& appSignal, QString appDataServiceId);
	void addSignals(const std::vector<AppSignalParam>& appSignals, QString appDataServiceId);

private:
	void addSignalPrivate(const AppSignalParam& appSignal, QString appDataServiceId);

public:
	std::vector<Hash> signalHashes() const;

	// Signal States
	//
	void invalidateSignalStates(Qt::HANDLE sourceThreadId);

	void setState(const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId);
	void setState(Hash signalHash, const AppSignalState& state, Qt::HANDLE sourceThreadId);
	void setState(const std::vector<AppSignalState>& states, Qt::HANDLE sourceThreadId);

	// Setpoints/Comparators
	//
	void setSetpoints(ComparatorSet&& setpoints);
	void setSetpoints(const ComparatorSet& setpoints);

	// Signals by sources (AppSignalIDs)
	//

	// Returns true if appDataServiceId has appSignalId
	//
	bool appDataServiceHasSignal(const QString& appDataServiceId, const QString& appSignalId) const;

	// IAppSignalManager implememntation - AppSignals
	//
	virtual int signalsCount() const final;
	virtual std::vector<AppSignalParam> signalList() const final;

	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const final;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const final;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const final;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const final;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const final;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const final;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const final;

	virtual QStringList signalTags(Hash signalHash) const final;
	virtual QStringList signalTags(const QString& appSignalId) const final;

	virtual bool signalHasTag(Hash signalHash, const QString& tag) const final;
	virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const final;

	virtual QStringList signalIdsByTag(const QString& tag) const final;

	virtual E::SignalType signalType(Hash signalHash, bool* found) const final;
	virtual E::SignalType signalType(const QString& appSignalId, bool* found) const final;

	virtual QString equipmentToAppSiganlId(const QString& equipmentId) const final;

	// IAppSignalManager implememntation - Setpoints
	//
	virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const final;

	// Tags
	//
	virtual QStringList tags() const final;

	// Extension
	//
public:
	AppSignalParam signalParamByEquipemntId(const QString& equipmentId, bool* found) const;

	void emitSignalParamsUpdated();

signals:
	void signalParamsUpdated();

	void addSignalToPriorityList(Hash signalHash) const;
	void addSignalsToPriorityList(QVector<Hash> signalHash) const;

protected slots:
	void configurationUpdated();

private:
	MonitorConfigController& m_configController;
	HasLogFile m_logFile;

	mutable QReadWriteLock m_paramsLocker;
	std::unordered_map<Hash, AppSignalParam, VoidHasher<Hash>> m_signalParams;	// Key is hash from AppSignalID (hash from hash here, not nice)
	std::unordered_map<QString, QString> m_signalParamByEquipmentId;			// Key is EquipmentId - value is AppSignalID
	std::unordered_map<QString, QStringList> m_tagToAppSignals;					// Key is tag - value is list of AppSignalIDs with this tag
	std::set<QString> m_tags;													// All tags for received AppSignals
	std::map<QString, std::set<QString>> m_appDataServiceToSignalLis;			// Key is AppDataServiceID, key is AppSignals received via this AppDataService

	mutable QReadWriteLock m_statesLocker;
	std::unordered_map<Hash, Sources, VoidHasher<Hash>> m_states;

	static constexpr qint64 MaxDiff = 1_sec;		//

	// ComparatorSet is threadsafe itself
	//
	ComparatorSet m_setpoints;
};


