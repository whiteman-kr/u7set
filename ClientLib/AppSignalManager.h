#pragma once

#include <chrono>
#include <map>
#include <set>
#include <memory>
#include <vector>
#include <unordered_map>
#include <QReadWriteLock>

#include "../lib/ISignalDataServer.h"
#include "../AppSignalLib/ComparatorSet.h"
#include "../AppSignalLib/IAppSignalManager.h"
#include "../UtilsLib/ILogFile.h"
#include "IAppSignalUpdater.h"
#include "IRecentAppSignals.h"


namespace ClientLib
{
	class RecentUsed
	{
	public:
		explicit RecentUsed(size_t maxSize = 500);

	public:
		void add(Hash h);
		void add(const std::vector<Hash>& hashes);

		bool remove(Hash hash);
		bool remove(const std::vector<Hash>& hashes);

		bool removeOutdated();

		[[nodiscard]] std::vector<Hash> hashes() const;

	private:
		const size_t m_maxSize{};
		std::map<Hash, qint64> m_signalToTime;				// key - signal hash, value - time of last update.
		std::multimap<qint64, Hash> m_timeToSignal;			// key - time of last update, value - signal hash.

		mutable QElapsedTimer m_lastTimeDataFetched;		// If data not fetched regulary, then ignore any add(...).

		static const int ExpiredTimeMs = 3000;				// If not fetch for this time, all cache is expired and cleared.
	};


	class AppSignalManager :
			public QObject,
			public IAppSignalManager,
			public IAppSignalUpdater,
			public IRecentAppSignals,
			public ISignalDataServer
	{
		Q_OBJECT

	public:
		explicit AppSignalManager(ILogFile* logFile, QObject* parent = nullptr);
		virtual ~AppSignalManager() = default;

		// IAppSignalUpdater implementation
		//
	public:
		virtual void reset() override;

		/// This should be called manually when all signal params are added.
		///
		virtual void notifySignalParamsUpdated() override;

		virtual void addSignal(const AppSignalParam& appSignal, const QString& appDataServiceId) override;
		virtual void addSignals(const std::vector<AppSignalParam>& appSignals, const QString& appDataServiceId) override;

		virtual void invalidateSignalStates(Qt::HANDLE sourceThreadId) override;

		virtual void setState(const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId) override;
		virtual void setState(Hash signalHash, const AppSignalState& state, Qt::HANDLE sourceThreadId) override;
		virtual void setState(const std::vector<AppSignalState>& states, Qt::HANDLE sourceThreadId) override;

	private:
		void addSignalPrivate(const AppSignalParam& appSignal, const QString& appDataServiceId);
		//
		// End of IAppSignalUpdater implementation

		// IRecentAppSignals implementation
		//
	public:
		virtual void addRecentAppSignal(Hash hash) override;
		virtual void addRecentAppSignals(const std::vector<Hash>& hashes) override;

		virtual std::vector<Hash> recentlyUsedAppSignals(const QString& appDataServivceId) override;

		//
		// End of IRecentAppSignals implementation

	public:
		std::vector<Hash> signalHashes() const;

		// Setpoints/Comparators
		//
		void setSetpoints(ComparatorSet&& setpoints);
		void setSetpoints(const ComparatorSet& setpoints);

		// IAppSignalManager implememntation - AppSignals
		//
		virtual int signalsCount() const override final;
		virtual std::vector<AppSignalParam> signalList() const override final;

		virtual bool signalExists(Hash hash) const override;
		virtual bool signalExists(const QString& appSignalId) const override final;
		virtual bool signalsExist(const QStringList& signalIds) const override final;

		virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override final;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override final;

		virtual AppSignalState signalState(Hash signalHash, bool* found) const override final;
		virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override final;

		virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override final;
		virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override final;

		virtual QStringList signalTags(Hash signalHash) const override final;
		virtual QStringList signalTags(const QString& appSignalId) const override final;

		virtual bool signalHasTag(Hash signalHash, const QString& tag) const override final;
		virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override final;

		virtual QStringList signalIdsByTag(const QString& tag) const override final;

		virtual E::SignalType signalType(Hash signalHash, bool* found) const override final;
		virtual E::SignalType signalType(const QString& appSignalId, bool* found) const override final;

		virtual QString equipmentToAppSiganlId(const QString& equipmentId) const override final;

		// IAppSignalManager implememntation - Setpoints
		//
		virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override final;

		//
		// ISignalDataServer implementation
		//

		/// Get AppDataService EquipmentIDs list by AppSignalID.
		///
		virtual QStringList dataServiceIds(const QString& appSignalId) const override;

		/// Return true if AppDataService contains signal.
		///
		virtual bool dataServiceHasSignal(const QString& serviceEquipmentId, const QString& appSignalId) const override;
		virtual bool dataServiceHasSignal(const QString& serviceEquipmentId, Hash signalHash) const override;

		// Tags
		//
		virtual QStringList tags() const override final;

		// Extension
		//
	public:
		AppSignalParam signalParamByEquipemntId(const QString& equipmentId, bool* found) const;

	signals:
		void signalParamsUpdated();

	private:
		HasLogFile m_logFile;

		struct SourceState
		{
			AppSignalState state{};
			Qt::HANDLE sourceThreadId{};
			std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{};	// State last time received or updated
		};

		struct Sources
		{
			size_t size = 0;
			std::array<SourceState, 4> sources{};	// 4 maximum possible channels of getting signal (2 regular, 2 recent)

			void set(const AppSignalState& state, Qt::HANDLE sourceThreadId);
			void invalidateSource(Qt::HANDLE sourceThreadId);

			[[nodiscard]] const AppSignalState& get() const;
		};

		mutable QReadWriteLock m_paramsLocker;
		std::unordered_map<Hash, AppSignalParam, VoidHasher<Hash>> m_signalParams;	// Key is hash from AppSignalID
		std::unordered_map<QString, QString> m_signalParamByEquipmentId;			// Key is EquipmentId - value is AppSignalID
		std::unordered_map<QString, QStringList> m_tagToAppSignals;					// Key is tag - value is list of AppSignalIDs with this tag
		std::set<QString> m_tags;													// All tags for received AppSignals
		std::map<QString, std::unordered_set<Hash>> m_appDataServiceToSignalHashList;// Key is AppDataServiceID, value is AppSignals received via this AppDataService

		mutable QReadWriteLock m_statesLocker;
		std::unordered_map<Hash, Sources, VoidHasher<Hash>> m_states;

		static constexpr qint64 MaxDiff = 1_sec;

		// ComparatorSet is threadsafe itself
		//
		ComparatorSet m_setpoints;

		mutable QMutex m_recentUsedMutex;	// It cannot be read/write locker, as every fetch the time insede RecentUsed is reset (what is write operation).
		RecentUsed m_recentUsed;
	};

}
