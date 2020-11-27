#include "SimTuningServiceCommunicator.h"
#include "Simulator.h"

namespace Sim
{
	TuningServiceCommunicator::TuningServiceCommunicator(Simulator* simulator, const ::TuningServiceSettings& settings) :
		m_simulator(simulator),
		m_log(m_simulator->log(), "TuningCommunicator"),
		m_settings(settings)
	{
		Q_ASSERT(simulator);

		connect(m_simulator, &Simulator::projectUpdated, this, &TuningServiceCommunicator::projectUpdated);

		return;
	}

	TuningServiceCommunicator::~TuningServiceCommunicator()
	{
	}

	bool TuningServiceCommunicator::startSimulation()
	{
		return true;
	}

	bool TuningServiceCommunicator::stopSimulation()
	{
		return true;
	}

	bool TuningServiceCommunicator::updateTuningRam(const QString& lmEquipmentId, const QString& portEquipmentId, const RamArea& ramArea, TimeStamp timeStamp)
	{
		qDebug() << "TuningServiceCommunicator::updateTuningRam LM: " << lmEquipmentId << ", Port: " << portEquipmentId << ", TimeStamp: " << timeStamp.toDateTime();

		// This function is called by LM after each workcycle
		// data: contains tuning memory area
		//
		int yuriy_beliy_to_do;	// Take ramArea and return as fast as possible
		return true;
	}

	void TuningServiceCommunicator::tuningModeChanged(const QString& lmEquipmentId, bool tuningMode)
	{
		qDebug() << "TuningServiceCommunicator::tuningModeChanged, LM: " << lmEquipmentId << ", value = " << tuningMode;
	}

	void TuningServiceCommunicator::writeTuningWord(const QString& lmEquipmentId, const QString& portEquipmentId, quint16 data, quint16 mask)
	{
		return writeTuningRecord(TuningRecord::createWord(lmEquipmentId, portEquipmentId, data, mask));
	}

	void TuningServiceCommunicator::writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 data)
	{
		return writeTuningRecord(TuningRecord::createDword(lmEquipmentId, portEquipmentId, data));
	}

	void TuningServiceCommunicator::writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, qint32 data)
	{
		return writeTuningRecord(TuningRecord::createSignedInt32(lmEquipmentId, portEquipmentId, data));
	}

	void TuningServiceCommunicator::writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, float data)
	{
		return writeTuningRecord(TuningRecord::createFloat(lmEquipmentId, portEquipmentId, data));
	}

	std::queue<TuningRecord> TuningServiceCommunicator::fetchWriteTuningQueue(const QString& lmEquipmentId)
	{
		std::queue<TuningRecord> result;

		QMutexLocker l(&m_qmutex);

		auto node = m_writeTuningQueue.extract(lmEquipmentId);
		if (node.empty() == false)
		{
			result = std::move(node.mapped());
		}

		return result;
	}

	void TuningServiceCommunicator::writeTuningRecord(TuningRecord&& r)
	{
		QMutexLocker l(&m_qmutex);

		m_writeTuningQueue[r.lmEquipmentId].push(std::move(r));

		return;
	}

	void TuningServiceCommunicator::projectUpdated()
	{
		// Project was loaded or cleared
		// Reset all queues here
		//
	}

	bool TuningServiceCommunicator::enabled() const
	{
		return m_enabled;
	}

	void TuningServiceCommunicator::TuningServiceCommunicator::setEnabled(bool value)
	{
		m_enabled = value;
	}

}


