#ifndef TUNINGSIGNALMANAGER_H
#define TUNINGSIGNALMANAGER_H

#include <unordered_map>
#include <condition_variable>
#include <QReadWriteLock>

#include "../AppSignalLib/ITuningSignalUpdater.h"
#include "../UtilsLib/ILogFile.h"
#include "ITuningSignalManager.h"
#include "TuningValue.h"

class TuningSignalManager :
		public QObject,
		public ITuningSignalManager,
		public ITuningSignalUpdater
{
	Q_OBJECT

public:
	explicit TuningSignalManager(const QString& clientEquipmentId, ILogFile* logFile, QObject* parent = nullptr);
	virtual ~TuningSignalManager();

public:
	bool load(const QByteArray& data);
	bool load(const ::Proto::AppSignalSet& message);

	// AppSignalParams
	//
	int signalsCount() const;
	std::vector<AppSignalParam> signalList() const;

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

	virtual TuningSignalState state(Hash hash, Hash tuningServiceHash, bool* found) const override;
	virtual TuningSignalState state(const QString& appSignalId, Hash tuningServiceHash, bool* found) const override;

	virtual QStringList signalIdsByTag(const QString& tag) const override;

	// Implementation ITuningSignalUpdater - State manipulation
	//
public:
	void reset() override;

	std::vector<Hash> signalHashes() const override;
	std::vector<Hash> signalHashes(const std::vector<Hash> lmEquipmentIdHashes) const override;

	void invalidateSignalStates(Hash tuningServiceHash) override;

	void setState(const TuningSignalState& state, Hash tuningServiceHash) override;
	void setStates(const std::vector<TuningSignalState>& states, Hash tuningServiceHash) override;

	bool waitForAllApplied(std::chrono::milliseconds timeout) const;

private:
	void notifySignalParamsUpdated() override;

	// End of ITuningSignalUpdater

public:
	// Unapplied values manipulation
	//
	void setUnappliedValue(Hash hash, const TuningValue& value);
	[[nodiscard]] TuningValue unappliedValue(Hash hash) const;

	[[nodiscard]] bool isUnapplied(Hash hash) const;


	// Signals
	//
signals:
	void signalsLoaded();			// Emited when new signals loaded

	// Data
	//
private:
	const Hash m_tuningClientHash = UNDEFINED_HASH;	// cached client hash value
	HasLogFile m_logFile;

	struct SourceState
	{
		TuningSignalState state{};
		Hash tuningServiceHash{UNDEFINED_HASH};
		std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{};	// State last time received or updated

		bool isUnapplied = false;
	};

	struct Sources
	{
		size_t size = 0;
		std::array<SourceState, 2> sources{};	// 2 maximum possible channels of getting signal

		TuningValue unappliedValue{};

		void set(const TuningSignalState& state, Hash tuningServiceHash);
		void invalidateSource(Hash tuningServiceHash);

		[[nodiscard]] const TuningSignalState& get() const;
		[[nodiscard]] const TuningSignalState& get(Hash tuningServiceHash, bool* found) const;

		// Working with unapplied values
		//
		void setUnappliedValue(const TuningValue& value);
		[[nodiscard]] const TuningValue& getUnappliedValue() const;

		[[nodiscard]] bool isValueUnapplied() const;						// Any source is unapplied
		[[nodiscard]] bool isValueUnapplied(Hash tuningServiceHash) const;	// Specified source is unapplied

		void setAsApplied(Hash tuningServiceHash);							// Set value as applied at specified source
	};

	// Objects storage
	//
	mutable QReadWriteLock m_signalsLock;							// For access to m_signals
	std::unordered_map<Hash, AppSignalParam> m_signals;
	std::unordered_map<QString, QStringList> m_tagToAppSignals;		// Key is tag - value is list of AppSignalIDs with this tag

	// States storage
	//
	mutable std::mutex m_statesMutex;								// For access to m_states and m_unappliedStates
	mutable std::condition_variable m_allStatesApplied;

	std::unordered_map<Hash, Sources, VoidHasher<Hash>> m_states;
	std::set<Hash> m_unappliedStates;
};


#endif // TUNINGSIGNALMANAGER_H
