#pragma once

#include "../UtilsLib/Address16.h"
#include "SimScopedLog.h"

#include "./include/Simulator/SimRam.h"
#include "./include/Simulator/SimOverrideSignals.h"


class QJSValue;
class QJSEngine;


namespace Sim
{
	class RamAreaInfo;
	class SimulatorPrivate;
	class AppSignalManagerImpl;

	class OverrideSignalsImpl : public QObject
	{
		Q_OBJECT

	public:
		explicit OverrideSignalsImpl(Sim::SimulatorPrivate* simulator, QObject* parent = nullptr);
		virtual ~OverrideSignalsImpl() = default;

	public:
		void clear();

		int addSignals(const QStringList& appSignalIds);
		bool addSignal(QString appSignalId, bool enabled, int index, OverrideSignalMethod method, QVariant value, QString script);

		void removeSignal(const QString& appSignalId);
		void removeSignals(const QStringList& appSignalIds);
		
		[[nodiscard]] bool containsSignal(const QString& appSignalId) const;

		void setEnable(QString appSignalId, bool enable);
		
		void setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value);
		void setValues(const std::vector<OverrideSetValueData>& overrideData);

		void updateSignals();								// Update signal descriptions, type, offsets, etc...

		bool runOverrideScripts(const QString& lmEquipmentId, qint64 workcycle);	// Runs override scripts and sets value to override signals
		void requestToResetOverrideScripts(const QString& lmEquipmentId);			// If module is reset, then script must be restarted, clear global variables, etc

		bool saveWorkspace(QString fileName) const;
		bool loadWorkspace(QString fileName);

		void updateRamOverrideData(const QString& lmEquipmentId, Sim::Ram& ram) const;

	signals:
		void signalsChanged(QStringList addedAppSignalIds);	// Added or deleted signal
		void stateChanged(QStringList appSignalIds);		// Changed value or enable state

	public:
		Sim::AppSignalManagerImpl& appSignalManager();
		const Sim::AppSignalManagerImpl& appSignalManager() const;

		std::optional<OverrideSignalParam> overrideSignal(QString appSignalId) const;
		std::vector<OverrideSignalParam> overrideSignals() const;
		QStringList overrideSignalIds() const;

		int changesCounter() const;

		std::vector<OverrideRamRecord> ramOverrideData(const QString& lmEquipmentId, const RamAreaInfo& ramAreaInfo) const;

	private:
		Sim::SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		mutable QReadWriteLock m_lock;
		std::map<QString, OverrideSignalParam> m_signals;	// Key is AppSignalID
		int m_changesCounter = 0;							// This variable is incremented every time m_signals has
															// any changes, so if it is changed then RAM requests update
	};

}
