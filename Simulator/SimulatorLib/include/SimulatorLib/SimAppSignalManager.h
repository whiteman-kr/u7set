#pragma once

#include "../AppSignalLib/IAppSignalManager.h"
#include <TrendView/TrendSignalState.h>

namespace Sim
{
	class AppSignalManagerImpl;
	class Ram;

	//
	//	AppSignalManager
	//
	class AppSignalManager final : public IAppSignalManager
	{
		friend class SimulatorPrivate;

		explicit AppSignalManager(AppSignalManagerImpl& impl);
		virtual ~AppSignalManager();

	public:
		QString ramDump(QString logicModuleId) const;

		void resetAll();
		void resetSignalParam();
		void resetRam();

		std::shared_ptr<TrendLib::RealtimeData> trendData(const QString& trendId,
														  const std::vector<Hash>& trendSignals,
														  TrendLib::TrendStateItem* minState,
														  TrendLib::TrendStateItem* maxState);

		std::optional<AppSignal> signalParamExt(const QString& appSignalId) const;
		std::optional<AppSignal> signalParamExt(Hash hash) const;

		Hash customToAppSignal(Hash customSignalHash) const;

		std::optional<AppSignalState> signalState(const QString& appSignalId, bool applyOverride) const;
		std::optional<AppSignalState> signalState(Hash signalHash, bool applyOverride) const; // <<<< GETTING STATE CODE HERE

		bool getUpdateForRam(const QString& equipmentId, Sim::Ram* ram) const;

	public:
		// Implementing IAppSignalManager - AppSignals
		//
		using ISignalManager::signalExists;
		using ISignalManager::signalParam;
		using IAppSignalManager::signalState;
		using IAppSignalManager::signalTags;
		using IAppSignalManager::signalHasTag;
		using IAppSignalManager::signalType;

		virtual int signalsCount() const override;
		virtual std::vector<Hash> signalHashes() const override;
		virtual std::vector<AppSignalParam> signalList() const override;

		virtual bool signalExists(Hash hash) const override;
		virtual bool signalsExist(const QStringList& signalIds) const override;

		virtual std::optional<AppSignalParam> signalParam(Hash signalHash) const override;

		virtual std::optional<AppSignalState> signalState(Hash signalHash) const override;
		virtual std::optional<AppSignalState> signalState(Hash signalHash, Hash dataServerHash) const override;

		virtual void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const override;
		virtual void signalState(std::span<const Hash> appSignalHashes,
								 Hash dataServerHash,
								 std::vector<std::optional<AppSignalState>>* result) const override;

		virtual QStringList signalTags(Hash signalHash) const override;

		virtual bool signalHasTag(Hash signalHash, const QString& tag) const override;

		virtual QStringList signalIdsByTag(const QString& tag) const override;

		virtual E::SignalType signalType(Hash signalHash, bool* found) const override;

		virtual QString equipmentToAppSignalId(const QString& equipmentId) const override;

		// Implementing IAppSignalManager - Setpoints/Comparators
		//
		virtual std::vector<std::shared_ptr<Comparator>> setpointsByInput(const QString& appSignalId) const override;
		virtual std::shared_ptr<Comparator> setpointByOutput(const QString& appSignalId) const override;

		// Implementing IAppSignalManager::tags
		//
		virtual QStringList tags() const override;

	private:
		AppSignalManagerImpl& m_impl;
	};
} // namespace Sim
