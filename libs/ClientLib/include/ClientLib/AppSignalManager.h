#pragma once

#include "../AppSignalLib/ComparatorSet.h"
#include "../AppSignalLib/IAppSignalManager.h"
#include "../AppSignalLib/RecentUsed.h"
#include "../UtilsLib/ILogFile.h"

#include "ISignalDataServer.h"
#include "IAppSignalUpdater.h"
#include "IRecentAppSignals.h"

#include <map>
#include <set>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QReadWriteLock>

namespace ClientLib
{
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
		virtual void addSignals(std::span<const AppSignalParam> appSignals, const QString& appDataServiceId) override;

		virtual void invalidateSignalStates(Qt::HANDLE sourceThreadId) override;

		virtual void setState(const QString& appSignalId, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId) override;
		virtual void setState(Hash signalHash, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId) override;
		virtual void setState(std::span<const AppSignalState> states, Hash dataServerHash, Qt::HANDLE sourceThreadId) override;

	private:
		void addSignalPrivate(const AppSignalParam& appSignal, const QString& appDataServiceId);
		//
		// End of IAppSignalUpdater implementation

		// IRecentAppSignals implementation
		//
	public:
		virtual void addRecentAppSignal(Hash hash) override;
		virtual void addRecentAppSignals(std::span<const Hash> hashes) override;

		virtual std::vector<Hash> recentlyUsedAppSignals(const QString& appDataServivceId) override;
		virtual bool hasRecentlyUsedAppSignals() override;

		//
		// End of IRecentAppSignals implementation

	public:
		std::vector<Hash> signalHashes() const;

		// Setpoints/Comparators
		//
		void setSetpoints(ComparatorSet&& setpoints);
		void setSetpoints(const ComparatorSet& setpoints);

		// IAppSignalManager implementation - AppSignals
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
		virtual AppSignalState signalState(Hash signalHash, Hash dataServerHash, bool* found) const override final;
		virtual AppSignalState signalState(const QString& appSignalId, const QString& dataServerId, bool* found) const override final;

		virtual void signalState(std::span<const Hash> appSignalHashes, std::vector<AppSignalState>* result, int* found) const override final;
		virtual void signalState(std::span<const QString> appSignalIds, std::vector<AppSignalState>* result, int* found) const override final;
		virtual void signalState(std::span<const Hash> appSignalHashes, Hash dataServerHash, std::vector<AppSignalState>* result, int* found) const override final;
		virtual void signalState(std::span<const QString> appSignalIds, const QString& dataServerId, std::vector<AppSignalState>* result, int* found) const override final;

		virtual QStringList signalTags(Hash signalHash) const override final;
		virtual QStringList signalTags(const QString& appSignalId) const override final;

		virtual bool signalHasTag(Hash signalHash, const QString& tag) const override final;
		virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override final;

		virtual QStringList signalIdsByTag(const QString& tag) const override final;

		virtual E::SignalType signalType(Hash signalHash, bool* found) const override final;
		virtual E::SignalType signalType(const QString& appSignalId, bool* found) const override final;

		virtual QString equipmentToAppSignalId(const QString& equipmentId) const override final;

		// IAppSignalManager implementation - Setpoints
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

		/// Extension, not part of ISignalDataServer, at least yet.
		///
		void filterByDataService(const QString& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const;

		/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
		///
		virtual std::vector<Hash> dataServiceSignals(const QString& serviceEquipmentId) const override;

		// Tags
		//
		virtual QStringList tags() const override final;

		// Extension
		//
	public:
		AppSignalParam signalParamByEquipemntId(const QString& equipmentId, bool* found) const;

	public:
		struct SourceState
		{
			AppSignalState state{};
			Hash dataServerHash{UNDEFINED_HASH};
			Qt::HANDLE sourceThreadId{};
			std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{};	// State last time received or updated
		};

		std::vector<SourceState> signalStateAllSources(const QString& appSignalId) const;

	signals:
		void signalParamsUpdated();

	private:
		struct Sources
		{
			size_t size = 0;
			std::array<SourceState, 4> sources{};	// 4 maximum possible channels of getting signal (2 regular, 2 recent)

			void set(const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId);
			void invalidateSource(Qt::HANDLE sourceThreadId);

			[[nodiscard]] const AppSignalState& get() const;
			[[nodiscard]] const AppSignalState& getForDataServer(Hash dataServerHash) const;
		};


		HasLogFile m_logFile;

		mutable QReadWriteLock m_paramsLocker;
		std::unordered_map<Hash, const AppSignalParam, VoidHasher<Hash>> m_signalParams;	// Key is hash from AppSignalID
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
		AppSignalLib::RecentUsed m_recentUsed;
	};

}
