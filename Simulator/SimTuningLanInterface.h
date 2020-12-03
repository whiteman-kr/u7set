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
		bool updateTuningRam(const Sim::RamArea& ramArea, TimeStamp timeStamp);
		void tuningModeChanged(bool tuningMode);

		std::queue<TuningRecord> fetchWriteTuningQueue();

	public:
	private:
		std::shared_ptr<Sim::TuningServiceCommunicator> m_tuningServiceCommunicator;
	};

}

