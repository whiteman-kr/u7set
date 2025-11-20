#pragma once

#include "../AppSignalLib/ComparatorSet.h"
#include "../AppSignalLib/IAppSignalManager.h"
#include "../AppSignalLib/RecentUsed.h"
#include "../UtilsLib/ILogFile.h"

#include "IAppSignalUpdater.h"
#include "IRecentAppSignals.h"
#include "ISignalDataServer.h"

#include <QReadWriteLock>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>


namespace ClientLib
{
	class AppSignalManager final : public QObject,
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

		virtual void addSignals(std::span<const AppSignalParam> appSignals, const QString& appDataServiceId) override;

		virtual void invalidateSignalStates(SourceIdType sourceThreadId) override;

		virtual void setStates(std::span<const AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId) override;

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
		// Setpoints/Comparators
		//
		void setSetpoints(ComparatorSet&& setpoints);
		void setSetpoints(const ComparatorSet& setpoints);

		// IAppSignalManager implementation - AppSignals
		//
		using ISignalManager::signalExists;
		using ISignalManager::signalParam;
		using IAppSignalManager::signalState;
		using IAppSignalManager::signalTags;
		using IAppSignalManager::signalHasTag;
		using IAppSignalManager::signalType;

		std::vector<Hash> signalHashes() const override;

		int signalsCount() const override;
		std::vector<AppSignalParam> signalList() const override;

		bool signalExists(Hash hash) const override;
		bool signalsExist(const QStringList& signalIds) const override;

		std::optional<AppSignalParam> signalParam(Hash signalHash) const override;

		std::optional<AppSignalState> signalState(Hash signalHash) const override;
		std::optional<AppSignalState> signalState(Hash signalHash, Hash dataServerHash) const override;

		void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const override;
		void signalState(std::span<const Hash> appSignalHashes,
						 Hash dataServerHash,
						 std::vector<std::optional<AppSignalState>>* result) const override;

		QStringList signalTags(Hash signalHash) const override;

		bool signalHasTag(Hash signalHash, const QString& tag) const override;

		QStringList signalIdsByTag(const QString& tag) const override;

		E::SignalType signalType(Hash signalHash, bool* found) const override;

		QString equipmentToAppSignalId(const QString& equipmentId) const override;

		// IAppSignalManager implementation - Setpoints
		//
		[[nodiscard]] std::vector<std::shared_ptr<Comparator>> setpointsByInput(const QString& appSignalId) const override;
		[[nodiscard]] std::shared_ptr<Comparator> setpointByOutput(const QString& appSignalId) const override;

		//
		// ISignalDataServer implementation
		//

		/// Get AppDataService EquipmentIDs list by AppSignalID.
		///
		QStringList dataServiceIds(const QString& appSignalId) const override;

		/// Return true if AppDataService contains signal.
		///
		bool dataServiceHasSignal(const QString& serviceEquipmentId, const QString& appSignalId) const override;
		bool dataServiceHasSignal(const QString& serviceEquipmentId, Hash signalHash) const override;

		/// Extension, not part of ISignalDataServer, at least yet.
		///
		void filterByDataService(const QString& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const;

		/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
		///
		std::vector<Hash> dataServiceSignals(const QString& serviceEquipmentId) const override;

		// Tags
		//
		QStringList tags() const override;

		// Extension
		//
	public:
		std::optional<AppSignalParam> signalParamByEquipmentId(const QString& equipmentId) const;

	public:
		struct SourceState
		{
			AppSignalState state{};
			Hash dataServerHash{UNDEFINED_HASH};
			SourceIdType sourceThreadId{};
			std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{}; // State last time received or updated
		};

		std::vector<SourceState> signalStateAllSources(const QString& appSignalId) const;

	signals:
		void signalParamsUpdated();

	private:
		struct Sources
		{
			size_t size = 0;
			std::array<SourceState, 4> sources{}; // 4 maximum possible channels of getting signal (2 regular, 2 recent)

			void set(const AppSignalState& state, Hash dataServerHash, SourceIdType sourceThreadId);
			void invalidateSource(SourceIdType sourceThreadId,
								  std::chrono::time_point<std::chrono::system_clock> now /* = std::chrono::system_clock::now()*/);

			[[nodiscard]] const AppSignalState& get() const;
			[[nodiscard]] const AppSignalState& getForDataServer(Hash dataServerHash) const;
		};


		HasLogFile m_logFile;

		mutable QReadWriteLock m_paramsLocker;
		std::unordered_map<Hash, const AppSignalParam, VoidHasher<Hash>> m_signalParams; // Key is hash from AppSignalID
		std::unordered_map<QString, QString> m_signalParamByEquipmentId;                 // Key is EquipmentId - value is AppSignalID
		std::unordered_map<QString, QStringList> m_tagToAppSignals; // Key is tag - value is list of AppSignalIDs with this tag
		std::set<QString> m_tags;                                   // All tags for received AppSignals
		std::map<QString, std::unordered_set<Hash>>
			m_appDataServiceToSignalHashList; // Key is AppDataServiceID, value is AppSignals received via this AppDataService

		mutable QReadWriteLock m_statesLocker;
		std::unordered_map<Hash, Sources, VoidHasher<Hash>> m_states;

		static constexpr qint64 MaxDiff = 1_sec;

		// ComparatorSet is threadsafe itself
		//
		ComparatorSet m_setpoints;

		mutable QMutex m_recentUsedMutex; // It cannot be read/write locker, as every fetch the time insede RecentUsed is reset (what is
										  // write operation).
		AppSignalLib::RecentUsed m_recentUsed;
	};

} // namespace ClientLib
