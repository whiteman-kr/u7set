#include "SimScriptLmDescription.h"

namespace Sim
{
	void ScriptLmDescription::setLmDescription(const LmDescription& lmDesc)
	{
		m_lmDesc = lmDesc;
	}

	bool ScriptLmDescription::isNull() const
	{
		return m_lmDesc.name().isEmpty();
	}

	bool ScriptLmDescription::isAddrInIoModuleBuf(int modulePlace, RamAddress addr) const
	{
		quint32 ioBufStartAddr = ioModuleBufStartAddr(modulePlace).bitAddress();

		return	addr.bitAddress() >= ioBufStartAddr &&
				addr.bitAddress() < ioBufStartAddr + ioModuleBufSize();
	}

	RamAddress ScriptLmDescription::ioModuleBufStartAddr(int modulePlace) const
	{
		Q_ASSERT(modulePlace > 0);

		RamAddress addr;

		addr.setOffset((modulePlace - 1) * ioModuleBufSize());
		addr.setBit(0);

		return addr;
	}

	int ScriptLmDescription::ioModuleBufSize() const
	{
		return m_lmDesc.memory().m_moduleDataSize;
	}
}

