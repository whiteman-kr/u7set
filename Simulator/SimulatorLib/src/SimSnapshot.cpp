#include "SimSnapshot.h"
#include "SimLogicModuleImpl.h"
#include "SimulatorPrivate.h"

#include <Simulator.pb.h>

namespace Sim
{
	Snapshot::Snapshot(ILogFile* logFile) :
		m_log{logFile, true, "Snapshot"}
	{
	}

	QByteArray Snapshot::take(QString snapshotId, SimulatorPrivate& simulator)
	{
		m_log.writeMessage("Taking snapshot " + snapshotId);

		// This lock ensures that no simulations can be started.
		// It may block all other threads that use 'control' while the snapshot is being taken.
		// m_controlDataMutex is a recursive mutex.
		//
		auto& control = simulator.control();

		std::unique_lock locker{control.m_controlDataMutex};

		ControlData cd = control.controlData();

		if (cd.m_state != SimControlState::Pause)
		{
			assert(cd.m_state == SimControlState::Pause);
			return {};
		}

		ProtoSim::Snapshot protoSnapshot;
		protoSnapshot.set_snapshotid(snapshotId.toStdString());
		protoSnapshot.set_projectname(simulator.projectName().toStdString());

		for (SimControlRunStruct& lm : cd.m_lms)
		{
			auto protoLm = protoSnapshot.add_logicmodules();
			bool ok = saveLogicModule(*protoLm, lm);
			if (ok == false)
			{
				m_log.writeMessage("Error saving LM state, EquipmentID " + lm->equipmentId());
				return {};
			}
		}

		// Serialize protoSnapshot to QByteArray
		//
		std::string serialized;
		if (protoSnapshot.SerializeToString(&serialized) == false)
		{
			m_log.writeMessage("Error snapshot proto-serialization.");
			return {};
		}

		return QByteArray::fromStdString(serialized);
	}

	bool Snapshot::apply(const QByteArray& snapshot, SimulatorPrivate& simulator)
	{
		m_log.writeMessage("Applying snapshot.");

		auto& control = simulator.control();
		std::lock_guard locker{control.m_controlDataMutex};

		if (control.m_controlData.m_state != SimControlState::Stop)
		{
			m_log.writeError("Cannot apply snapshot: simulation is not stopped.");
			return false;
		}

		// Wait when simulation thread exit from simulation loop (Sim::ControlImpl::processRun).
		//
		control.m_insideProcessRun.wait(true);

		// Parse data.
		//
		ProtoSim::Snapshot protoSnapshot;
		bool ok = protoSnapshot.ParseFromArray(snapshot.constData(), snapshot.size());

		if (ok == false)
		{
			m_log.writeError("Snapshot data corrupted.");
			return false;
		}

		QString snapshotId = QString::fromStdString(protoSnapshot.snapshotid());
		m_log.writeText("Restoring snapshot: " + snapshotId);

		QString projectName = QString::fromStdString(protoSnapshot.projectname());
		if (simulator.projectName() != projectName)
		{
			m_log.writeError(
				QString{"Project name mismatch, loaded project: %1, snapshot project: %2"}.arg(simulator.projectName()).arg(projectName));
			return false;
		}

		// Set run list
		//
		QStringList logicModules;
		logicModules.reserve(protoSnapshot.logicmodules_size());

		std::transform(protoSnapshot.logicmodules().cbegin(),
					   protoSnapshot.logicmodules().cend(),
					   std::back_inserter(logicModules),
					   [](const auto& protoLm)
					   {
						   return QString::fromStdString(protoLm.equipmentid());
					   });

		control.setRunList(logicModules);

		// startSimulation is required to fill LMs, as the mutex still locked, no actual simulation is happening.
		//
		ok = control.startSimulation();
		if (ok == false)
		{
			m_log.writeError("Failed to start simulation after restoring snapshot");
			control.stop();
			return false;
		}

		control.pause();

		// Simulation is not running, so we can safely apply snapshot data.
		//
		ok = applyPrivate(protoSnapshot, simulator);

		if (ok == false)
		{
			m_log.writeError("Failed to restore snapshot");
			control.stop();
			return false;
		}

		m_log.writeMessage(QString{"Snapshot %1 restored"}.arg(snapshotId));
		return true;
	}

	bool Snapshot::saveLogicModuleRam(::ProtoSim::SnapshotLmRam& protoRam, const Sim::Ram& ram)
	{
		for (const RamArea& area : ram.m_memoryAreas)
		{
			auto protoArea = protoRam.add_memoryareas();

			// RamAreaInfo
			//
			protoArea->set_name(area.name().toStdString());
			protoArea->set_access(static_cast<int32_t>(area.access()));
			protoArea->set_offset(static_cast<uint64_t>(area.offset()));
			protoArea->set_size(static_cast<uint64_t>(area.size()));

			// RamArea
			//
			protoArea->set_clearonstartcycle(area.m_clearOnStartCycle);
			protoArea->set_data(area.m_data.constData(), area.m_data.size());

			for (const OverrideRamRecord& ord : area.m_overrideData)
			{
				auto protoOrd = protoArea->add_overridedata();
				protoOrd->set_mask(ord.mask);
				protoOrd->set_data(ord.data);
			}
		}

		protoRam.set_overridesignalslastcounter(ram.m_overrideSignalsLastCounter);

		return true;
	}

	bool Snapshot::saveLogicModule(::ProtoSim::SnapshotLogicModule& protoLm, const Sim::SimControlRunStruct& sclm)
	{
		assert(sclm.m_lm != nullptr);

		protoLm.set_laststarttime(sclm.m_lastStartTime.count());
		protoLm.set_possibletoadvanceto(sclm.m_possibleToAdvanceTo.count());
		protoLm.set_cyclescounter(sclm.m_cyclesCounter);

		const LogicModuleImpl* lm = sclm.m_lm.get();

		protoLm.set_equipmentid(lm->equipmentId().toStdString());

		protoLm.set_tuningeepromcrc(lm->tuningEeprom().crc32(true));
		protoLm.set_confeepromcrc(lm->confEeprom().crc32(true));
		protoLm.set_applogiceepromcrc(lm->appLogicEeprom().crc32(true));

		protoLm.set_runtimemode(static_cast<int32_t>(lm->runtimeMode()));
		protoLm.set_devicestate2(static_cast<int32_t>(lm->deviceState()));

		bool ok = saveLogicModuleRam(*protoLm.mutable_ram(), lm->ram());
		if (ok == false)
		{
			return false;
		}

		// optional SnapshotLogicUnitData m_logicUnit = 25;
		// repeated SnapshotConnection m_connections = 30;

		// repeated AfbComponent afbs = 40;

		// Tuning
		//
		protoLm.set_armingkey(lm->armingKey());
		protoLm.set_tuningkey(lm->tuningKey());

		// optional SnapshotLmRamArea m_tuningRamArea = 60;

		protoLm.set_sorisset(lm->sorIsSet());

		protoLm.set_sorsetswitch1(lm->sorSetSwitch1());
		protoLm.set_sorsetswitch2(lm->sorSetSwitch2());
		protoLm.set_sorsetswitch3(lm->sorSetSwitch3());

		return true;
	}

	bool Snapshot::applyPrivate(const ::ProtoSim::Snapshot& protoSnapshot, SimulatorPrivate& simulator)
	{
		auto& control = simulator.control();

		// Restore LMs' states.
		//
		if (control.m_controlData.m_lms.size() != protoSnapshot.logicmodules_size())
		{
			m_log.writeError(QString{"LogicModule count mismatch. Project has %1 LMs, snapshot has %2 LMs."}
								 .arg(control.m_controlData.m_lms.size())
								 .arg(protoSnapshot.logicmodules_size()));
			return false;
		}

		bool ok = true;
		for (auto& lm : control.m_controlData.m_lms)
		{
			auto it = std::find_if(protoSnapshot.logicmodules().cbegin(),
								   protoSnapshot.logicmodules().cend(),
								   [&lm](const ProtoSim::SnapshotLogicModule& protoLm)
								   {
									   auto id = QString::fromStdString(protoLm.equipmentid());
									   return id == lm->equipmentId();
								   });

			if (it == protoSnapshot.logicmodules().cend())
			{
				m_log.writeError("Cannot find state of LM " + lm->equipmentId());
				ok = false;
				continue;
			}

			bool lmRestoreOk = restoreLogicModule(*it, lm);
			if (lmRestoreOk == false)
			{
				m_log.writeError("Failed to update LogicModule with the provided snapshot data. LM: " + lm->equipmentId());
				ok = false;
				continue;
			}
		}

		if (ok == false)
		{
			return false;
		}

		ok = restoreAppSignalManager(simulator);

		return ok;
	}

	bool Snapshot::restoreLogicModuleRam(const ::ProtoSim::SnapshotLmRam& protoRam, Sim::Ram& ram)
	{
		std::vector<RamArea*> ramAreas = ram.memoryAreas();

		if (protoRam.memoryareas_size() != ramAreas.size())
		{
			m_log.writeError(QString{"RamArea size mismatch, LM has %1, snapshot has %2 ram areas."}
								 .arg(ramAreas.size())
								 .arg(protoRam.memoryareas_size()));
			return false;
		}

		for (const auto& protoRamArea : protoRam.memoryareas())
		{
			auto name = QString::fromStdString(protoRamArea.name());
			auto it = find_if(ramAreas.cbegin(),
							  ramAreas.cend(),
							  [&name](auto lmRamArea)
							  {
								  return name == lmRamArea->name();
							  });

			if (it == ramAreas.cend())
			{
				m_log.writeError(QString{"Cannot find ram area '%1'"}.arg(name));
				return false;
			}

			RamArea* ramArea = *it;

			if (ramArea->access() != static_cast<E::LogicModuleRamAccess>(protoRamArea.access()) ||
				ramArea->offset() != protoRamArea.offset() || ramArea->size() != protoRamArea.size())
			{
				m_log.writeError(QString{"LogicModule RamAres %1 has parameters mismatch."}.arg(name));
				return false;
			}

			ramArea->m_clearOnStartCycle = protoRamArea.clearonstartcycle();
			ramArea->m_data = QByteArray{protoRamArea.data()};

			ramArea->m_overrideData.clear();
			ramArea->m_overrideData.reserve(protoRamArea.overridedata_size());
			for (const auto& protoOrd : protoRamArea.overridedata())
			{
				ramArea->m_overrideData.emplace_back(static_cast<quint16>(protoOrd.mask()), static_cast<quint16>(protoOrd.data()));
			}
		}

		ram.m_overrideSignalsLastCounter = protoRam.overridesignalslastcounter();

		return true;
	}

	bool Snapshot::restoreLogicModule(const ::ProtoSim::SnapshotLogicModule& protoLm, Sim::SimControlRunStruct& sclm)
	{
		assert(sclm.m_lm != nullptr);

		m_log.writeMessage(QString{"Restoring LogicModule %1..."}.arg(sclm->equipmentId()));

		sclm.m_lastStartTime = std::chrono::microseconds{protoLm.laststarttime()};
		sclm.m_possibleToAdvanceTo = std::chrono::microseconds{protoLm.possibletoadvanceto()};
		sclm.m_cyclesCounter = protoLm.cyclescounter();

		LogicModuleImpl* lm = sclm.m_lm.get();

		auto equipmentId = QString::fromStdString(protoLm.equipmentid());
		if (lm->equipmentId() != equipmentId)
		{
			m_log.writeError(QString{"Wrong LM EquipmentID, actual: %1, expected: %2."}.arg(lm->equipmentId(), equipmentId));
			return false;
		}

		if (lm->tuningEeprom().crc32(true) != protoLm.tuningeepromcrc())
		{
			m_log.writeError(QString{"Wrong tuning EEPROM CRC, actual: %1, expected: %2."}
								 .arg(lm->tuningEeprom().crc32(true))
								 .arg(protoLm.tuningeepromcrc()));
			return false;
		}

		if (lm->confEeprom().crc32(true) != protoLm.confeepromcrc())
		{
			m_log.writeError(QString{"Wrong config EEPROM CRC, actual: %1, expected: %2."}
								 .arg(lm->confEeprom().crc32(true))
								 .arg(protoLm.confeepromcrc()));
			return false;
		}

		if (lm->appLogicEeprom().crc32(true) != protoLm.applogiceepromcrc())
		{
			m_log.writeError(QString{"Wrong AppLogic EEPROM CRC, actual: %1, expected: %2."}
								 .arg(lm->appLogicEeprom().crc32(true))
								 .arg(protoLm.applogiceepromcrc()));
			return false;
		}

		// Restore LM state.
		//
		lm->m_device.setRuntimeMode(static_cast<RuntimeMode>(protoLm.runtimemode()));
		lm->m_device.m_deviceState2.store(static_cast<DeviceState>(protoLm.devicestate2()));

		bool ok = restoreLogicModuleRam(protoLm.ram(), lm->mutableRam());
		if (ok == false)
		{
			m_log.writeError(QString{"Restoring LogicModule RAM failed, LM: %1"}.arg(lm->equipmentId()));
			return false;
		}

		// Restore external keys.
		//
		lm->setArmingKey(protoLm.armingkey());
		lm->setTuningKey(protoLm.tuningkey());

		lm->m_device.setSorIsSet(protoLm.sorisset());
		lm->m_device.setSorSetSwitch1(protoLm.sorsetswitch1());
		lm->m_device.setSorSetSwitch2(protoLm.sorsetswitch2());
		lm->m_device.setSorSetSwitch3(protoLm.sorsetswitch3());

		return true;
	}

	bool Snapshot::restoreAppSignalManager(SimulatorPrivate& simulator)
	{
		// Set LogicModule's RAM to Sim::AppSignalManager
		//
		auto& control = simulator.control();

		auto ms = control.controlData().m_currentTime;
		QDateTime currentDateTime = control.controlData().currentTime();

		TimeStamp plantTime{ms.count() + currentDateTime.offsetFromUtc() * 1000};
		TimeStamp localTime{plantTime};
		TimeStamp systemTime{ms.count()};

		for (auto& lm : control.m_controlData.m_lms)
		{
			simulator.appSignalManager().setData(lm.equipmentId(), lm->ram(), plantTime, localTime, systemTime);
		}

		return true;
	}
} // namespace Sim