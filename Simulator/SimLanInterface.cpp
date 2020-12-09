#include "SimLanInterface.h"
#include "Simulator.h"

namespace Sim
{
	LanInterface::LanInterface(const::LanControllerInfo& lci, Lans* lans) :
		m_lans(lans),
		m_log(lans->log()),
		m_lanControllerInfo(lci)
	{
		Q_ASSERT(lans);
		return;
	}

	LanInterface::~LanInterface()
	{
	}

	const QString& LanInterface::lmEquipmentId() const
	{
		return m_lans->logicModuleId();
	}

	const QString& LanInterface::portEquipmentId() const
	{
		return m_lanControllerInfo.equipmentID;
	}

	bool LanInterface::enabled() const
	{
		return m_enabled.load(std::memory_order::memory_order_relaxed);
	}

	void LanInterface::setEnabled(bool value)
	{
		m_enabled = value;
	}

	bool LanInterface::isTuning() const
	{
		return (static_cast<int>(m_lanControllerInfo.lanControllerType) & static_cast<int>(E::LanControllerType::Tuning)) != 0;
	}

	TuningLanInterface* LanInterface::toTuningLanInterface()
	{
		if (isTuning() == false)
		{
			Q_ASSERT(isTuning());
			return nullptr;
		}

		TuningLanInterface* r = dynamic_cast<TuningLanInterface*>(this);
		Q_ASSERT(r);

		return r;
	}

	const TuningLanInterface* LanInterface::toTuningLanInterface() const
	{
		if (isTuning() == false)
		{
			Q_ASSERT(isTuning());
			return nullptr;
		}

		const TuningLanInterface* r = dynamic_cast<const TuningLanInterface*>(this);
		Q_ASSERT(r);

		return r;
	}

	bool LanInterface::isAppData() const
	{
		return (static_cast<int>(m_lanControllerInfo.lanControllerType) & static_cast<int>(E::LanControllerType::AppData)) != 0;
	}

	int LanInterface::appDataSizeBytes() const
	{
		return m_lanControllerInfo.appDataSizeBytes;
	}

	bool LanInterface::isDiagData() const
	{
		return (static_cast<int>(m_lanControllerInfo.lanControllerType) & static_cast<int>(E::LanControllerType::DiagData)) != 0;
	}

}
