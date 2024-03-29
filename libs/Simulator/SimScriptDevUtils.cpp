#include <SimulatorPrivate.h>
#include "../UtilsLib/WUtils.h"

namespace Sim
{

	ScriptDevUtils::ScriptDevUtils()
	{
		qDebug() << "ScriptDevUtils::ScriptDevUtils()";
	}

	ScriptDevUtils::ScriptDevUtils(SimulatorPrivate* simulator) :
		m_simulator(simulator)
	{
		qDebug() << "ScriptDevUtils::ScriptDevUtils(SimulatorPrivate* simulator)";
		assert(m_simulator);
	}

	bool ScriptDevUtils::addrInIoModuleBuf(QString lmEquipmentId, quint32 modulePlace, RamAddress addr) const
	{
		std::shared_ptr<LogicModuleImpl> lm = m_simulator->logicModule(lmEquipmentId);

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
		std::shared_ptr<LogicModuleImpl> lm = m_simulator->logicModule(lmEquipmentId);

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
		std::shared_ptr<LogicModuleImpl> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return RamAddress::BadAddress;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		return	lmDescription.memory().m_appLogicWordDataOffset;
	}

	quint32 ScriptDevUtils::reverseUnsignedInt32(quint32 v) const
	{
		return reverseUint32(v);
	}

	qint32 ScriptDevUtils::reverseSignedInt32(quint32 v) const
	{
		return reverseInt32(v);
	}

	quint16 ScriptDevUtils::reverseUnsignedInt16(quint16 v) const
	{
		return reverseUint16(v);
	}

	qint16 ScriptDevUtils::reverseSignedInt16(quint16 v) const
	{
		return reverseInt16(v);
	}

	float ScriptDevUtils::reverseFloat(float v) const
	{
		return ::reverseFloat(v);
	}
}
