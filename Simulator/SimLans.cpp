#include "SimLans.h"
#include "Simulator.h"
#include "SimAppDataLanInterface.h"
#include "SimDiagDataLanInterface.h"

namespace Sim
{
	Lans::Lans(DeviceEmulator* logicModuleDevice, Simulator* simulator) :
		m_logicModuleDevice(logicModuleDevice),
		m_simulator(simulator),
		m_log(simulator->log(), "LAN")
	{
	}

	Lans::~Lans()
	{
	}

	void Lans::clear()
	{
		m_interfaces.clear();
	}

	bool Lans::init(const ::LogicModuleInfo& logicModuleInfo)
	{
		for (const ::LanControllerInfo& lc : logicModuleInfo.lanControllers)
		{
			if (lc.isTuning() == true)
			{
				std::shared_ptr<TuningServiceCommunicator> tuningServiceCommunicator;

				if (lc.tuningEnable == true)
				{
					tuningServiceCommunicator = m_simulator->software().tuningService(lc.tuningServiceID);

					if (tuningServiceCommunicator == nullptr)
					{
						log().writeAlert(QString("TuningService %1 not forund for LAN port %2")
										 .arg(lc.tuningServiceID)
										 .arg(lc.equipmentID));
					}
				}

				auto i = std::make_unique<TuningLanInterface>(lc, this, tuningServiceCommunicator);
				m_interfaces.emplace_back(std::move(i));
			}

			if (lc.isAppData() == true)
			{
				auto i = std::make_unique<AppDataLanInterface>(lc, this);
				m_interfaces.emplace_back(std::move(i));
			}

			if (lc.isDiagData() == true)
			{
				auto i = std::make_unique<DiagDataLanInterface>(lc, this);
				m_interfaces.emplace_back(std::move(i));
			}
		}

		return true;
	}

	bool Lans::isAppDataEnabled() const
	{
		if (m_simulator->software().enabled() == false)
		{
			return false;
		}

		// true if at least one LAN can transmit app data
		//
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isAppData() == true && i->enabled() == true && i->appDataSizeBytes() > 0)
			{
				return true;
			}
		}

		return false;
	}

	bool Lans::sendAppData(const QByteArray& data, TimeStamp timeStamp)
	{
		if (m_simulator->software().enabled() == false)
		{
			return false;
		}

		// true if at least one LAN can transmit app data
		//
		bool ok = true;

		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isAppData() == true && i->enabled() == true && i->appDataSizeBytes() > 0)
			{
				ok &= m_simulator->software().sendAppData(logicModuleId(), i->portEquipmentId(), data, timeStamp);
			}
		}

		return ok;
	}

	bool Lans::isTuningEnabled() const
	{
		if (m_simulator->software().enabled() == false)
		{
			return false;
		}

		// true if at least one LAN can transmit app data
		//
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true &&
				i->enabled() == true &&
				i->toTuningLanInterface() &&
				i->toTuningLanInterface()->enabled() == true)
			{
				return true;
			}
		}

		return false;
	}

	bool Lans::updateTuningRam(const RamArea& data, bool setSorChassisState, TimeStamp timeStamp)
	{
		if (m_simulator->software().enabled() == false ||
			m_logicModuleDevice->runtimeMode() != RuntimeMode::TuningMode)
		{
			return false;
		}

		// true if at least one LAN can transmit tuning data
		//
		bool ok = true;

		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				TuningLanInterface* tli = i->toTuningLanInterface();
				if (tli == nullptr)
				{
					continue;
				}

				ok &= tli->updateTuningRam(data, setSorChassisState, timeStamp);
			}
		}

		return ok;
	}

	void Lans::tuningModeEntered(const RamArea& data, bool setSorChassisState, TimeStamp timeStamp)
	{
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				TuningLanInterface* tli = i->toTuningLanInterface();
				if (tli == nullptr)
				{
					continue;
				}

				tli->tuningModeEntered(data, setSorChassisState, timeStamp);
			}
		}
	}

	void Lans::tuningModeLeft()
	{
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				TuningLanInterface* tli = i->toTuningLanInterface();
				if (tli == nullptr)
				{
					continue;
				}

				tli->tuningModeLeft();
			}
		}
	}

	std::queue<TuningRecord> Lans::fetchWriteTuningQueue()
	{
		std::queue<TuningRecord> result;

		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				TuningLanInterface* tli = i->toTuningLanInterface();
				if (tli == nullptr)
				{
					continue;
				}

				if (result.empty() == true)
				{
					result = tli->fetchWriteTuningQueue();
				}
				else
				{
					// No stl alg to concat two queues ((
					//
					std::queue<TuningRecord> src = tli->fetchWriteTuningQueue();

					while (src.empty() == false)
					{
						result.push(src.front());
						src.pop();
					}
				}
			}
		}

		return result;
	}

	void Lans::sendTuningWriteConfirmation(std::vector<qint64> confirmedRecords, const Sim::RamArea& data, bool setSorChassisState, TimeStamp timeStamp)
	{
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				TuningLanInterface* tli = i->toTuningLanInterface();
				if (tli == nullptr)
				{
					continue;
				}

				tli->sendWriteConfirmation(confirmedRecords, data, setSorChassisState, timeStamp);
			}
		}

		return;
	}

	ScopedLog& Lans::log()
	{
		return m_log;
	}

	const QString& Lans::logicModuleId() const
	{
		Q_ASSERT(m_logicModuleDevice);
		return m_logicModuleDevice->equipmentId();
	}
}