#pragma once

#include "queue"
#include "SimLanInterface.h"
#include "../lib/TimeStamp.h"
#include "SimTuningRecord.h"

namespace Sim
{
	class Simulator;
	class RamArea;
	class TuningServiceCommunicator;


	class TuningLanInterface : public LanInterface
	{
		Q_OBJECT

	public:
		explicit TuningLanInterface(const ::LanControllerInfo& lci,
									Lans* lans,
									std::shared_ptr<TuningServiceCommunicator> tuningServiceCommunicator);
		virtual ~TuningLanInterface();

	public:
		bool updateTuningRam(const Sim::RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp);

		void tuningModeEntered(const Sim::RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp);
		void tuningModeLeft();

		std::queue<TuningRecord> fetchWriteTuningQueue();
		void sendWriteConfirmation(std::vector<qint64> confirmedRecords, const Sim::RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp);

	public:
	private:
		std::shared_ptr<Sim::TuningServiceCommunicator> m_tuningServiceCommunicator;
	};

}

