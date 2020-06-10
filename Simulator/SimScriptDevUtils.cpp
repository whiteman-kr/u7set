#include "SimScriptDevUtils.h"
#include <Simulator.h>

namespace Sim
{

	ScriptDevUtils::ScriptDevUtils()
	{
		qDebug() << "ScriptDevUtils::ScriptDevUtils()";
	}

	ScriptDevUtils::ScriptDevUtils(Simulator* simulator) :
		m_simulator(simulator)
	{
		qDebug() << "ScriptDevUtils::ScriptDevUtils(Simulator* simulator)";
		assert(m_simulator);
	}

	bool ScriptDevUtils::addrInIoModuleBuf(QString lmEquipmentId, quint32 modulePlace, RamAddress addr) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return false;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		if (modulePlace < 1 || modulePlace > lmDescription.memory().m_moduleCount)
		{
			return false;
		}

		return	addr.offset() >= (lmDescription.memory().m_moduleDataOffset + (modulePlace - 1) * lmDescription.memory().m_moduleDataSize) &&
				addr.offset() < (lmDescription.memory().m_moduleDataOffset + modulePlace * lmDescription.memory().m_moduleDataSize);
	}

	bool ScriptDevUtils::addrInRegBuf(QString lmEquipmentId, RamAddress addr) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return false;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		return	addr.offset() >= lmDescription.memory().m_appLogicWordDataOffset &&
				addr.offset() < lmDescription.memory().m_appLogicWordDataOffset + lmDescription.memory().m_appLogicWordDataSize;
	}

	quint32 ScriptDevUtils::regBufStartAddr(QString lmEquipmentId) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return RamAddress::BadAddress;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		return	lmDescription.memory().m_appLogicWordDataOffset;
	}

}
