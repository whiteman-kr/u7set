#pragma once
#include "SimScopedLog.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/TimeStamp.h"
#include "SimTuningRecord.h"

namespace Sim
{
	class Simulator;
	class RamArea;

	class TuningServiceCommunicator : public QObject
	{
		Q_OBJECT

	public:
		TuningServiceCommunicator(Simulator* simulator, const TuningServiceSettings& settings);
		virtual ~TuningServiceCommunicator();

	public:
		bool startSimulation();
		bool stopSimulation();

		// This function is called by Simulator to provide current RAM state of Tuning memory area if sLM is in TuningMode and Tuning is enabled.
		// Data is in LogicMoudule's native endianness (BE).
		//
		bool updateTuningRam(const QString& lmEquipmentId, const QString& portEquipmentId, const RamArea& ramArea, TimeStamp timeStamp);

		// This function is called by Simulator when module's tuning mode has changed to or from TuningMode
		//
		void tuningModeChanged(const QString& lmEquipmentId, bool tuningMode);

	public:
		void writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, quint32 data, quint32 mask);
		void writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, qint32 data);
		void writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, float data);

		std::queue<TuningRecord> fetchWriteTuningQueue(const QString& lmEquipmentId);
	private:
		void writeTuningRecord(TuningRecord&& r);

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool enabled() const;
		void setEnabled(bool value);

		// Data Section
		//
	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		::TuningServiceSettings m_settings;

		std::atomic<bool> m_enabled{true};		// Allow communication to TuningService

		// Queue to write data to LogicModule
		// Key is LM EquipmentID
		//
		QMutex m_qmutex;
		std::map<QString, std::queue<TuningRecord>> m_writeTuningQueue;
	};

}


