#pragma once
#include "SimOutput.h"
#include "../lib/TimeStamp.h"

namespace Sim
{
	class Simulator;

	class AppDataTransmitter : public QObject, protected Output
	{
		Q_OBJECT

	public:
		AppDataTransmitter(Simulator* simulator);
		virtual ~AppDataTransmitter();

	public:
		bool startSimulation();
		bool stopSimulation();
		bool sendData(const QString& lmEquipmentId, QByteArray&& data, TimeStamp timeStamp);

	protected slots:
		void projectUpdated();		// Project was loaded or cleared

	public:
		bool enabled() const;
		void setEnabled(bool value);

	private:
		Simulator* m_simulator;

		std::atomic<bool> m_enabled{false};			// Allow AppData trasmittion to AppDataSrv
	};

}




