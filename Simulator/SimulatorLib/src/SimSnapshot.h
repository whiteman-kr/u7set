#pragma once
#include "SimScopedLog.h"


namespace ProtoSim
{
	class Snapshot;
	class SnapshotLmRam;
	class SnapshotLogicModule;
} // namespace ProtoSim


namespace Sim
{
	class Ram;
	class SimulatorPrivate;
	struct SimControlRunStruct;


	class Snapshot
	{
	public:
		explicit Snapshot(ILogFile* logFile);

		// Take snapshot
		//
	public:
		QByteArray take(QString snapshotId, SimulatorPrivate& simulator);
		bool apply(const QByteArray& snapshot, SimulatorPrivate& simulator);

	private:
		bool saveLogicModuleRam(::ProtoSim::SnapshotLmRam& protoRam, const Sim::Ram& ram);
		bool saveLogicModule(::ProtoSim::SnapshotLogicModule& protoLm, const Sim::SimControlRunStruct& sclm);

		// Apply-restore snapshot
		//
	private:
		bool applyPrivate(const ::ProtoSim::Snapshot& protoSnapshot, SimulatorPrivate& simulator);

		bool restoreLogicModuleRam(const ::ProtoSim::SnapshotLmRam& protoRam, Sim::Ram& ram);
		bool restoreLogicModule(const ::ProtoSim::SnapshotLogicModule& protoLm, Sim::SimControlRunStruct& sclm);
		bool restoreAppSignalManager(SimulatorPrivate& simulator);

	private:
		mutable ScopedLog m_log;
	};
} // namespace Sim