#pragma once
#include "SimScopedLog.h"


namespace ProtoSim
{
	class AfbComponentSet;
	class Snapshot;
	class SnapshotLmRam;
	class SnapshotLogicModule;
} // namespace ProtoSim


namespace Sim
{
	class AfbComponentSet;
	class ConnectionsImpl;
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
		bool saveLogicModuleAfbs(::ProtoSim::AfbComponentSet& protoAfbSet, const Sim::AfbComponentSet& afbSet);
		bool saveLogicModuleRam(::ProtoSim::SnapshotLmRam& protoRam, const Sim::Ram& ram);
		bool saveLogicModule(::ProtoSim::SnapshotLogicModule& protoLm, const Sim::SimControlRunStruct& sclm);
		bool saveConnections(::ProtoSim::Snapshot& protoSnapshot, const Sim::ConnectionsImpl& connections);

		// Apply-restore snapshot
		//
	private:
		bool applyPrivate(const ::ProtoSim::Snapshot& protoSnapshot, SimulatorPrivate& simulator);

		bool restoreLogicModuleAfbs(const ::ProtoSim::AfbComponentSet& protoAfbSet, Sim::AfbComponentSet& afbSet);
		bool restoreLogicModuleRam(const ::ProtoSim::SnapshotLmRam& protoRam, Sim::Ram& ram);
		bool restoreLogicModule(const ::ProtoSim::SnapshotLogicModule& protoLm, Sim::SimControlRunStruct& sclm);
		bool restoreAppSignalManager(SimulatorPrivate& simulator);
		bool restoreConnections(const ::ProtoSim::Snapshot& protoSnapshot,
								Sim::ConnectionsImpl& connections,
								std::chrono::microseconds currentTime);

	private:
		mutable ScopedLog m_log;
	};
} // namespace Sim