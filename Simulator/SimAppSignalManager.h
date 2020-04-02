#ifndef SIMAPPSIGNALMANAGER_H
#define SIMAPPSIGNALMANAGER_H

#include <memory>
#include <map>
#include <unordered_map>
#include <optional>
#include <QHash>
#include <QReadWriteLock>
#include "../lib/Signal.h"
#include "../lib/AppSignalManager.h"
#include "SimRam.h"
#include "SimOutput.h"

namespace Sim
{
	class Simulator;

	class AppSignalManager : public IAppSignalManager, protected Sim::Output
	{
	public:
		explicit AppSignalManager(Simulator* simulator, QObject* parent = nullptr);
		virtual ~AppSignalManager();

	public:
		void reset();
		bool load(QString fileName);

		void setData(const QString& equipmentId, const Sim::Ram& ram);

		std::optional<Signal> signalParamExt(const QString& appSignalId) const;

	public:
		// Implementing IAppSignalManager - AppSignals
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

		// Implementing IAppSignalManager - Setpoints/Comparators
		//
		virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override;

		// Data
		//
	private:
		Simulator* m_simulator = nullptr;

		mutable QReadWriteLock m_signalParamLock;
		std::unordered_map<Hash, AppSignalParam> m_signalParams;
		std::unordered_map<Hash, Signal> m_signalParamsExt;			// Except AppSignalParam, we need Signal as it has more information (like offset in memory)

		// SimRuntime data
		//
		mutable QReadWriteLock m_ramLock;
		std::map<QString, Ram> m_ram;		// key is EquipmentID
	};

}

#endif // SIMAPPSIGNALMANAGER_H
