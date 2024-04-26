#pragma once

#include <HardwareLib/LogicModulesInfo.h>
#include "../CommonLib/Times.h"
#include <Simulator/SimRam.h>

#include "SimScopedLog.h"
#include "SimTuningRecord.h"
#include "SimLanInterface.h"

namespace Sim
{
	class SimulatorPrivate;
	class DeviceEmulator;

	//
	// This class contains a set of LANs for LogicModule
	//
	class Lans
	{
	public:
		explicit Lans(DeviceEmulator* logicModuleDevice, SimulatorPrivate* simulator);
		virtual ~Lans();

	public:
		void clear();
		bool init(const ::LogicModuleInfo& logicModuleInfo);

		// AppData
		//
		bool isAppDataEnabled() const;
		bool sendAppData(const QByteArray& data, TimeStamp timeStamp);

		// DiagData
		//

		// Tuning
		//
		bool isTuningEnabled() const;

		bool updateTuningRam(const Sim::RamArea& data, bool setSorChassisState, TimeStamp timeStamp);	// Copy of tuning RAM Area

		void tuningModeEntered(const Sim::RamArea& data, bool setSorChassisState, TimeStamp timeStamp);
		void tuningModeLeft();

		std::queue<TuningRecord> fetchWriteTuningQueue();

		void sendTuningWriteConfirmation(const QString &portEquipmentId,
										 qint64 confirmedRecordId,
										 const Sim::RamArea& data,
										 bool setSorChassisState,
										 TimeStamp timeStamp);
	public:
		ScopedLog& log();

		const QString& logicModuleId() const;

	private:
		DeviceEmulator* m_logicModuleDevice;
		SimulatorPrivate* m_simulator;
		mutable ScopedLog m_log;

		std::vector<std::unique_ptr<LanInterface>> m_interfaces;
	};

}


