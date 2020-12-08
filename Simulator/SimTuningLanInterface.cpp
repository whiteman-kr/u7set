#include "SimTuningLanInterface.h"
#include "SimTuningServiceCommunicator.h"
#include "Simulator.h"

namespace Sim
{

	TuningLanInterface::TuningLanInterface(const::LanControllerInfo& lci,
										   Lans* lans,
										   std::shared_ptr<TuningServiceCommunicator> tuningServiceCommunicator) :
		LanInterface(lci, lans),
		m_tuningServiceCommunicator(tuningServiceCommunicator)
	{
		m_log.setOutputScope("TuningLanInterface");

		setEnabled(lci.tuningEnable);

		if (lci.tuningEnable == true)
		{
			Q_ASSERT(m_tuningServiceCommunicator);
		}

		return;
	}

	TuningLanInterface::~TuningLanInterface()
	{
	}

	bool TuningLanInterface::updateTuningRam(const RamArea& ramArea, TimeStamp timeStamp)
	{
		if (enabled() == false || m_tuningServiceCommunicator == nullptr)
		{
			return false;
		}

		return m_tuningServiceCommunicator->updateTuningRam(lmEquipmentId(), portEquipmentId(), ramArea, timeStamp);
	}

	void TuningLanInterface::tuningModeEntered(const RamArea& ramArea, TimeStamp timeStamp)
	{
		if (m_tuningServiceCommunicator == nullptr)
		{
			return;
		}

		return m_tuningServiceCommunicator->tuningModeEntered(lmEquipmentId(), portEquipmentId(), ramArea, timeStamp);
	}

	void TuningLanInterface::tuningModeLeft()
	{
		if (m_tuningServiceCommunicator == nullptr)
		{
			return;
		}

		return m_tuningServiceCommunicator->tuningModeLeft(lmEquipmentId(), portEquipmentId());
	}

	std::queue<TuningRecord> TuningLanInterface::fetchWriteTuningQueue()
	{
		std::queue<TuningRecord> result;

		if (m_tuningServiceCommunicator != nullptr)
		{
			result = m_tuningServiceCommunicator->fetchWriteTuningQueue(lmEquipmentId());
		}

		return result;
	}
}
