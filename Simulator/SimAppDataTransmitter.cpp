#include "SimAppDataTransmitter.h"
#include "Simulator.h"

namespace Sim
{

	AppDataTransmitter::AppDataTransmitter(Simulator* simulator) :
		Output("AppDataTransmitter"),
		m_simulator(simulator)
	{
		Q_ASSERT(m_simulator);

		connect(m_simulator, &Simulator::projectUpdated, this, &AppDataTransmitter::projectUpdated);
	}

	AppDataTransmitter::~AppDataTransmitter()
	{
	}

	bool AppDataTransmitter::startSimulation()
	{
		return true;
	}

	bool AppDataTransmitter::stopSimulation()
	{
		return true;
	}

	bool AppDataTransmitter::sendData(const QString& lmEquipmentId, QByteArray&& data, TimeStamp timeStamp)
	{
		//qDebug() << "Send reg data from " << lmEquipmentId << " data size" << data.size() << " time " << timeStamp.toDateTime();

		//std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);
		//if (lm == nullptr)
		//{
		//	Q_ASSERT(lm);
		//	writeError("LogicModule %1 is not found");
		//	return false;
		//}
		// const ::LogicModuleInfo& lmi = lm->logicModuleExtraInfo();
		Q_UNUSED(data);
		Q_UNUSED(lmEquipmentId);
		Q_UNUSED(timeStamp);

		return true;
	}

	void AppDataTransmitter::projectUpdated()
	{
		// Project was loaded or cleared
		// Reset all queues here
		//
	}

}
