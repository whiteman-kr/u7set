#pragma once
#include "../lib/SoftwareXmlReader.h"
#include "SimAppDataTransmitter.h"
#include "SimTuningServiceCommunicator.h"

namespace  Sim
{

	class Software
	{
	public:
		Software(Simulator* simulator);

	public:
		void clear();
		bool loadSoftwareXml(QString fileName);

		bool startSimulation();
		bool stopSimulation();

		// AppDataService
		//
		bool sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp);

		// TuningService Communatation
		//
		std::shared_ptr<Sim::TuningServiceCommunicator> tuningService(QString equipmentId) const;

	public:
		[[nodiscard]] bool enabled() const;
		void setEnabled(bool value);

		[[nodiscard]] Sim::AppDataTransmitter& appDataTransmitter();
		[[nodiscard]] const Sim::AppDataTransmitter& appDataTransmitter() const;

	private:
		Simulator* m_simulator = nullptr;

		std::vector<SoftwareXmlInfo> m_software;

		std::atomic<bool> m_enabled{false};

		// Sends data to AppDataService
		//
		Sim::AppDataTransmitter m_appDataTransmitter;

		// TunningService Communicators, each instance works with its own TuningService
		// Key is TuningService equipmentId
		//
		std::map<QString, std::shared_ptr<Sim::TuningServiceCommunicator>> m_tuningServiceCommunicators;
	};

}


