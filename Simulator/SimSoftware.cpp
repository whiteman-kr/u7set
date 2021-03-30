#include "SimSoftware.h"

namespace  Sim
{

	Software::Software(Simulator* simulator) :
		m_simulator(simulator),
		m_appDataTransmitter{simulator}
	{
		Q_ASSERT(m_simulator);
	}

	void Software::clear()
	{
		m_software.clear();
		m_tuningServiceCommunicators.clear();

		return;
	}

	bool Software::loadSoftwareXml(QString fileName)
	{
		clear();

		// Raad and parse file /Common/Software.xml
		//
		::SoftwareXmlReader reader;

		bool ok = reader.readSoftwareXml(fileName);
		if (ok == false)
		{
			return false;
		}

		// --
		//
		const std::map<QString, SoftwareXmlInfo>& sxi = reader.softwareXmlInfo();
		m_software.reserve(sxi.size());

		for (auto&[equipmentId, si] : sxi)
		{
			Q_ASSERT(equipmentId == si.equipmentID);

			if (si.softwareType() == E::SoftwareType::TuningService)
			{
				std::shared_ptr<Sim::TuningServiceCommunicator> tsc = std::make_shared<Sim::TuningServiceCommunicator>(m_simulator, equipmentId);

				m_tuningServiceCommunicators.emplace(equipmentId, std::move(tsc));
			}

			m_software.emplace_back(std::move(si));
		}

		return ok;
	}

	bool Software::startSimulation(QString profileName)
	{
		bool ok = true;

		ok &= m_appDataTransmitter.startSimulation(profileName);

		for (auto&[id, tcs] : m_tuningServiceCommunicators)
		{
			Q_UNUSED(id);

			ok &= tcs->startSimulation(profileName);
		}

		return ok;
	}

	bool Software::stopSimulation()
	{
		bool ok = true;

		ok &= m_appDataTransmitter.stopSimulation();

		for (auto&[id, tcs] : m_tuningServiceCommunicators)
		{
			Q_UNUSED(id);

			ok &= tcs->stopSimulation();
		}

		return ok;
	}

	bool Software::sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp)
	{
		if (enabled() == false)
		{
			return true;
		}

		return m_appDataTransmitter.sendData(lmEquipmentId, portEquipmentId, data, timeStamp);
	}

	std::shared_ptr<Sim::TuningServiceCommunicator> Software::tuningService(QString equipmentId) const
	{
		std::shared_ptr<Sim::TuningServiceCommunicator> result;

		auto it = m_tuningServiceCommunicators.find(equipmentId);
		if (it != m_tuningServiceCommunicators.end())
		{
			result = it->second;
		}

		return result;
	}

	bool Software::enabled() const
	{
		return m_enabled.load(std::memory_order::relaxed);
	}

	void Software::setEnabled(bool value)
	{
		m_enabled = value;
	}

	Sim::AppDataTransmitter& Software::appDataTransmitter()
	{
		return m_appDataTransmitter;
	}

	const Sim::AppDataTransmitter& Software::appDataTransmitter() const
	{
		return m_appDataTransmitter;
	}

}
