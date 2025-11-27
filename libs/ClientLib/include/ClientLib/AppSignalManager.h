#pragma once


#include "../AppSignalLib/ComparatorSet.h"
#include "../AppSignalLib/IAppSignalManager.h"
#include "../AppSignalLib/ISignalManager.h"
#include "../AppSignalLib/RecentUsed.h"
#include "../UtilsLib/ILogFile.h"

#include <AppSignalLibStd/AppSignalManagerCore.h>
#include <AppSignalLibStd/IAppSignalUpdater.h>
#include <AppSignalLibStd/IRecentAppSignals.h>
#include <AppSignalLibStd/ISignalDataServer.h>

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

		virtual void addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId) override;

		virtual void invalidateSignalStates(SourceIdType sourceThreadId) override;

		virtual void setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId) override;

	private:
		void addSignalPrivate(const AppSignalParam& appSignal, const QString& appDataServiceId);
		//
		// End of IAppSignalUpdater implementation

		// IRecentAppSignals implementation
		//
	public:
		virtual void addRecentAppSignal(Hash hash) override;
		virtual void addRecentAppSignals(std::span<const Hash> hashes) override;

		virtual std::vector<Hash> recentlyUsedAppSignals(const std::string& appDataServivceId) override;
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
		std::vector<std::string> dataServiceIds(const std::string& appSignalId) const override;

		/// Return true if AppDataService contains signal.
		///
		bool dataServiceHasSignal(const std::string& serviceEquipmentId, const std::string& appSignalId) const override;
		bool dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const override;

		/// Extension, not part of ISignalDataServer, at least yet.
		///
		void filterByDataService(const QString& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const;

		/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
		///
		std::vector<Hash> dataServiceSignals(const std::string& serviceEquipmentId) const override;

		// Tags
		//
		QStringList tags() const override;

		// Extension
		//
	public:
		std::optional<AppSignalParam> signalParamByEquipmentId(const QString& equipmentId) const;

	private:
		using CoreType = AppSignalStdLib::AppSignalManagerCore<AppSignalParam, AppSignalState, QString, QStringList>;

	public:
		using SourceState = CoreType::SourceState;
		std::vector<SourceState> signalStateAllSources(const QString& appSignalId) const;

	signals:
		void signalParamsUpdated();

	private:
		HasLogFile m_logFile;

		CoreType m_core;

		// ComparatorSet is threadsafe itself
		//
		ComparatorSet m_setpoints;

		mutable QMutex m_recentUsedMutex; // It cannot be read/write locker, as every fetch the time insede RecentUsed is reset (what is
										  // write operation).
		AppSignalLib::RecentUsed m_recentUsed;
	};

} // namespace ClientLib
