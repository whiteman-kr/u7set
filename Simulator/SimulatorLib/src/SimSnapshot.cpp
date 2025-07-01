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

		QElapsedTimer timer;
		timer.start();

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

		// Save LMs.
		//
		protoSnapshot.mutable_logicmodules()->Reserve(static_cast<int>(cd.m_lms.size()));

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

		// Save connections.
		//
		bool saveConnOk = saveConnections(protoSnapshot, simulator.connections());
		if (saveConnOk == false)
		{
			m_log.writeMessage("Error saving connections.");
			return {};
		}

		// Serialize protoSnapshot to QByteArray
		//
		QByteArray snapshotData;
		snapshotData.resizeForOverwrite(protoSnapshot.ByteSizeLong());

		if (protoSnapshot.SerializeToArray(snapshotData.data(), snapshotData.size()) == false)
		{
			m_log.writeMessage("Error snapshot proto-serialization.");
			return {};
		}

		m_log.writeMessage(QString("Snapshot %1 taken in %2 ms").arg(snapshotId).arg(timer.elapsed()));

		return snapshotData;
	}

	bool Snapshot::apply(const QByteArray& snapshot, SimulatorPrivate& simulator)
	{
		m_log.writeMessage("Applying snapshot.");

		QElapsedTimer timer;
		timer.start();

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

		auto timeElapsed = timer.elapsed();

		m_log.writeMessage(QString{"Snapshot %1 restored in %2 ms"}.arg(snapshotId).arg(timeElapsed));
		return true;
	}

	bool Snapshot::saveLogicModuleAfbs(::ProtoSim::AfbComponentSet& protoAfbSet, const Sim::AfbComponentSet& afbSet)
	{
		protoAfbSet.mutable_afbs()->Reserve(static_cast<int>(afbSet.m_components.size()));

		for (const auto& afb : afbSet.m_components)
		{
			auto protoAfb = protoAfbSet.add_afbs();
			protoAfb->mutable_instances()->Reserve(static_cast<int>(afb.m_instances.size()));

			for (const auto& afbInstance : afb.m_instances)
			{
				auto protoInstance = protoAfb->add_instances();
				protoInstance->set_instanceno(afbInstance.m_instanceNo);
				protoInstance->set_versionopindex(afbInstance.m_versionOpIndex);

				protoInstance->mutable_params()->Reserve(static_cast<int>(afbInstance.m_params_a.size()));

				for (const auto& afbParam : afbInstance.m_params_a)
				{
					auto protoAfbParam = protoInstance->add_params();

					// Do not save default values, it reduces file size several times.
					//
					if (auto data = afbParam.uint64Value(); data != std::numeric_limits<quint64>::max())
					{
						protoAfbParam->set_data(data);
					}

					if (afbParam.m_paramOpIndex != 0xFFFF)
					{
						protoAfbParam->set_paramopindex(afbParam.m_paramOpIndex);
					}
				}
			}
		}

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

		ok = saveLogicModuleAfbs(*protoLm.mutable_afbset(), lm->m_device.m_afbComponents);
		if (ok == false)
		{
			return false;
		}

		// optional SnapshotLogicUnitData m_logicUnit = 25;
		// repeated SnapshotConnection m_connections = 30;

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

	bool Snapshot::saveConnections(::ProtoSim::Snapshot& protoSnapshot, const Sim::ConnectionsImpl& connections)
	{
		auto allConnections = connections.connections();

		auto protoConnections = protoSnapshot.mutable_connections();
		protoConnections->Reserve(static_cast<int>(allConnections.size()));

		for (const auto& conn : allConnections)
		{
			auto protoConn = protoConnections->Add();

			protoConn->set_id(conn->connectionId().toStdString());

			protoConn->set_enable(conn->enabled());
			protoConn->set_timeout(conn->timeout());

			for (const auto& port : conn->ports())
			{
				if (port.portInfo().portNo == 1)
				{
					protoConn->set_port1equipmentid(port.portInfo().equipmentID.toStdString());
					continue;
				}

				if (port.portInfo().portNo == 2)
				{
					protoConn->set_port2equipmentid(port.portInfo().equipmentID.toStdString());
					continue;
				}

				m_log.writeError(QString{"Wrong port number, it can be internal error, ConnectionID %1, Port %2"}
									 .arg(conn->connectionId())
									 .arg(port.portInfo().portNo));
				return false;
			}

			// Set port1 data
			//
			{
				auto proPort1SentData = protoConn->mutable_port1sentdata();
				proPort1SentData->set_data(conn->m_port1sentData.m_data.data(), conn->m_port1sentData.m_data.size());
				proPort1SentData->set_senttime(conn->m_port1sentData.m_sentTime.count());

				protoConn->set_port1receivebuffer(conn->m_port1receiveBuffer.data(), conn->m_port1receiveBuffer.size());
				protoConn->set_port1sendbuffer(conn->m_port1sendBuffer.data(), conn->m_port1sendBuffer.size());
			}

			// Set port2 data
			//
			{
				auto proPort2SentData = protoConn->mutable_port2sentdata();
				proPort2SentData->set_data(conn->m_port2sentData.m_data.data(), conn->m_port2sentData.m_data.size());
				proPort2SentData->set_senttime(conn->m_port2sentData.m_sentTime.count());

				protoConn->set_port2receivebuffer(conn->m_port2receiveBuffer.data(), conn->m_port2receiveBuffer.size());
				protoConn->set_port2sendbuffer(conn->m_port2sendBuffer.data(), conn->m_port2sendBuffer.size());
			}
		}

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

		// Restore AppSignalManager state.
		//
		ok = restoreAppSignalManager(simulator);
		if (ok == false)
		{
			m_log.writeError("Failed to restore AppSignalManager state from snapshot.");
			return false;
		}

		// Restore Connections.
		//
		auto currentTime = simulator.control().controlData().m_currentTime;
		ok = restoreConnections(protoSnapshot, simulator.connections(), currentTime);
		if (ok == false)
		{
			m_log.writeError("Failed to restore connections from snapshot.");
			return false;
		}

		return ok;
	}

	bool Snapshot::restoreLogicModuleAfbs(const ::ProtoSim::AfbComponentSet& protoAfbSet, Sim::AfbComponentSet& afbSet)
	{
		if (protoAfbSet.afbs_size() != afbSet.m_components.size())
		{
			m_log.writeError(QString{"AfbComponentSet size mismatch, LM has %1, snapshot has %2 AfbComponents."}
								 .arg(afbSet.m_components.size())
								 .arg(protoAfbSet.afbs_size()));
			return false;
		}

		auto restoreAfbParamFunc = [](const ::ProtoSim::AfbComponentParam& protoAfbParam, Sim::AfbComponentParam& afbParam)
		{
			afbParam.setUint64Value(protoAfbParam.data());
			afbParam.m_paramOpIndex = protoAfbParam.paramopindex();
		};

		for (size_t compIndex = 0; compIndex < protoAfbSet.afbs_size(); ++compIndex)
		{
			auto& protoAfb = protoAfbSet.afbs(static_cast<int>(compIndex));
			auto& afbComponent = afbSet.m_components[compIndex];

			if (protoAfb.instances_size() != afbComponent.m_instances.size())
			{
				m_log.writeError(QString{"AfbComponent %1 has %2 instances, snapshot has %3 instances."}
									 .arg(compIndex)
									 .arg(afbComponent.m_instances.size())
									 .arg(protoAfb.instances_size()));
				return false;
			}

			for (size_t instanceIndex = 0; instanceIndex < protoAfb.instances_size(); ++instanceIndex)
			{
				auto& protoInstance = protoAfb.instances(static_cast<int>(instanceIndex));
				auto& afbInstance = afbComponent.m_instances[instanceIndex];

				if (protoInstance.instanceno() != afbInstance.m_instanceNo)
				{
					m_log.writeError(QString{"AfbComponent %1 instance %2 has instance number mismatch, actual: %3, expected: %4."}
										 .arg(compIndex)
										 .arg(instanceIndex)
										 .arg(afbInstance.m_instanceNo)
										 .arg(protoInstance.instanceno()));
					return false;
				}

				if (protoInstance.params_size() != afbInstance.m_params_a.size())
				{
					m_log.writeError(QString{"AfbComponent %1 instance %2 has parameters size mismatch, actual: %3, expected: %4."}
										 .arg(compIndex)
										 .arg(instanceIndex)
										 .arg(afbInstance.m_params_a.size())
										 .arg(protoInstance.params_size()));
					return false;
				}

				assert(afbInstance.m_versionOpIndex == protoInstance.versionopindex());

				for (size_t paramIndex = 0; paramIndex < protoInstance.params_size(); ++paramIndex)
				{
					auto& afbParam = afbInstance.m_params_a[paramIndex];
					restoreAfbParamFunc(protoInstance.params(static_cast<int>(paramIndex)), afbParam);
				}
			}
		}

		return true;
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

		ok = restoreLogicModuleAfbs(protoLm.afbset(), lm->m_device.m_afbComponents);
		if (ok == false)
		{
			m_log.writeError(QString{"Restoring LogicModule AfbComponentSet failed, LM: %1"}.arg(lm->equipmentId()));
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
	bool Snapshot::restoreConnections(const ::ProtoSim::Snapshot& protoSnapshot,
									  Sim::ConnectionsImpl& connections,
									  std::chrono::microseconds currentTime)
	{
		for (const auto& protoConn : protoSnapshot.connections())
		{
			auto connectionId = QString::fromStdString(protoConn.id());
			m_log.writeMessage(QString{"Restoring connection %1..."}.arg(connectionId));

			auto conn = connections.connection(connectionId);
			if (conn == nullptr)
			{
				m_log.writeError(QString{"Connection %1 does not exist."}.arg(connectionId));
				return false;
			}

			conn->setEnabled(protoConn.enable());
			conn->m_timeout.store(protoConn.timeout());

			// Check ports.
			//
			auto ports = conn->ports();

			if (auto port1EquipmentId = QString::fromStdString(protoConn.port1equipmentid()); port1EquipmentId.isEmpty() == false)
			{
				bool present = std::any_of(ports.begin(),
										   ports.end(),
										   [&port1EquipmentId](const Sim::ConnectionPort& port)
										   {
											   return port.portInfo().equipmentID == port1EquipmentId;
										   });
				if (present == false)
				{
					m_log.writeError(QString{"Connection %1 port1 %2 mismatch."}.arg(connectionId).arg(port1EquipmentId));
					return false;
				}
			}

			if (auto port2EquipmentId = QString::fromStdString(protoConn.port2equipmentid()); port2EquipmentId.isEmpty() == false)
			{
				bool present = std::any_of(ports.begin(),
										   ports.end(),
										   [&port2EquipmentId](const Sim::ConnectionPort& port)
										   {
											   return port.portInfo().equipmentID == port2EquipmentId;
										   });
				if (present == false)
				{
					m_log.writeError(QString{"Connection %1 port2 %2 mismatch."}.arg(connectionId).arg(port2EquipmentId));
					return false;
				}
			}

			// Restore port data.
			//
			conn->m_port1sentData.m_data.assign(protoConn.port1sentdata().data().begin(), protoConn.port1sentdata().data().end());
			conn->m_port1sentData.m_sentTime = currentTime; // Even though sent time is stored, set current time to avoid timeout.
			conn->m_port1receiveBuffer.assign(protoConn.port1receivebuffer().begin(), protoConn.port1receivebuffer().end());
			conn->m_port1sendBuffer.assign(protoConn.port1sendbuffer().begin(), protoConn.port1sendbuffer().end());

			conn->m_port2sentData.m_data.assign(protoConn.port2sentdata().data().begin(), protoConn.port2sentdata().data().end());
			conn->m_port2sentData.m_sentTime = currentTime; // Even though sent time is stored, set current time to avoid timeout.
			conn->m_port2receiveBuffer.assign(protoConn.port2receivebuffer().begin(), protoConn.port2receivebuffer().end());
			conn->m_port2sendBuffer.assign(protoConn.port2sendbuffer().begin(), protoConn.port2sendbuffer().end());
		}

		return true;
	}
} // namespace Sim