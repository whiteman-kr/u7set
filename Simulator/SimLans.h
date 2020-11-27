#pragma once

#include "../lib/TimeStamp.h"
#include "../lib/LogicModulesInfo.h"
#include "SimScopedLog.h"
#include "SimTuningLanInterface.h"

namespace Sim
{
	class Simulator;
	class DeviceEmulator;

	//
	// This class contains a set of LANs for LogicModule
	//
	class Lans
	{
	public:
		explicit Lans(DeviceEmulator* logicModuleDevice, Simulator* simulator);
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

		bool updateTuningRam(const Sim::RamArea& data, TimeStamp timeStamp);	// Copy of tuning RAM Area
		void tuningModeChanged(bool tuningMode);

	public:
		ScopedLog& log();

		const QString& logicModuleId() const;

	private:
		DeviceEmulator* m_logicModuleDevice;
		Simulator* m_simulator;
		mutable ScopedLog m_log;

		std::vector<std::unique_ptr<LanInterface>> m_interfaces;
	};

}


