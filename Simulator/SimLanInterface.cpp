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

	QString LanInterface::lmEquipmentId() const
	{
		return m_lans->logicModuleId();
	}

	QString LanInterface::portEquipmentId() const
	{
		return m_lanControllerInfo.equipmentID;
	}

	bool LanInterface::enabled() const
	{
		return m_enabled;
	}

	void LanInterface::setEnabled(bool value)
	{
		m_enabled = value;
	}

	bool LanInterface::isTuning() const
	{
		return (static_cast<int>(m_lanControllerInfo.lanControllerType) & static_cast<int>(E::LanControllerType::Tuning)) != 0;
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
