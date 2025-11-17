#pragma once

#include "../AppSignalLib/IAppSignalManager.h"

namespace ReportLib
{
	//
	//
	// ReportAppSignalProvider - this class is used to provide app signals for drawing schemas, showing and getting signal ids, description,
	// precision, etc...
	//
	//
	class ReportAppSignalProvider final : public IAppSignalManager
	{
	public:
		ReportAppSignalProvider() = delete;
		ReportAppSignalProvider(const AppSignalSet* signalSet);

		// IAppSignalManager implementation
		//
	public:
		using ISignalManager::signalExists;
		using ISignalManager::signalParam;
		using IAppSignalManager::signalState;
		using IAppSignalManager::signalTags;
		using IAppSignalManager::signalHasTag;
		using IAppSignalManager::signalType;

		int signalsCount() const override;
		std::vector<Hash> signalHashes() const override;
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

		// Setpoints
		//
		std::vector<std::shared_ptr<Comparator>> setpointsByInput(const QString& appSignalId) const override;
		std::shared_ptr<Comparator> setpointByOutput(const QString& appSignalId) const override;

		// Tags
		//
		virtual QStringList tags() const override;

	private:
		const AppSignalSet* m_signalSet = nullptr;
	};
} // namespace ReportLib
