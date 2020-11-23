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
				auto i = std::make_unique<TuningLanInterface>(lc, this);
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
		if (m_simulator->appDataTransmitter().enabled() == false)
		{
			return false;
		}

		// true if at least one LAN can transmit app data
		//
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isAppData() == true && i->enabled() == true)
			{
				return true;
			}
		}

		return false;
	}

	bool Lans::sendAppDataData(const QByteArray& data, TimeStamp timeStamp)
	{
		if (m_simulator->appDataTransmitter().enabled() == false)
		{
			return false;
		}

		// true if at least one LAN can transmit app data
		//
		bool ok = true;

		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isAppData() == true && i->enabled() == true)
			{
				ok &= m_simulator->appDataTransmitter().sendData(logicModuleId(), i->portEquipmentId(), data, timeStamp);
			}
		}

		return ok;
	}

	bool Lans::isTuningEnabled() const
	{
		int check_global_tuning_enable_flag;
//		if (m_simulator->appDataTransmitter().enabled() == false)
//		{
//			return false;
//		}

		// true if at least one LAN can transmit app data
		//
		for (const std::unique_ptr<LanInterface>& i : m_interfaces)
		{
			if (i->isTuning() == true && i->enabled() == true)
			{
				return true;
			}
		}

		return false;
	}

	ScopedLog& Lans::log()
	{
		return m_log;
	}

	QString Lans::logicModuleId() const
	{
		Q_ASSERT(m_logicModuleDevice);
		return m_logicModuleDevice->equipmentId();
	}
}
