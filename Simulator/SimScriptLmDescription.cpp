#include "SimScriptLmDescription.h"

namespace Sim
{
	void ScriptLmDescription::setLogicModule(std::shared_ptr<LogicModule> lm)
	{
		m_lmDesc = lm->lmDescription();
		m_lmInfo = lm->logicModuleExtraInfo();
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

	bool ScriptLmDescription::isAddrInAcquiredAppDataBuf(RamAddress addr) const
	{
		RamAddress adStartAddr(appDataStartAddr());

		RamAddress appDataEndAddr;

		appDataEndAddr.setOffset(adStartAddr.offset() + appDataSizeW());
		appDataEndAddr.setBit(0);

		return	addr.bitAddress() >= adStartAddr.bitAddress() &&
				addr.bitAddress() < appDataEndAddr.bitAddress();
	}

	bool ScriptLmDescription::isAddrAfterAcquiredAppDataBuf(RamAddress addr) const
	{
		RamAddress appDataEndAddr;

		appDataEndAddr.setOffset(appDataStartAddr().offset() + appDataSizeW());
		appDataEndAddr.setBit(0);

		return	addr.bitAddress() >= appDataEndAddr.bitAddress();
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

	RamAddress ScriptLmDescription::appDataStartAddr() const
	{
		RamAddress appDataStartAddr;

		appDataStartAddr.setOffset(m_lmDesc.memory().m_appLogicWordDataOffset);
		appDataStartAddr.setBit(0);

		return appDataStartAddr;
	}

	int ScriptLmDescription::appDataSizeW() const
	{
		return m_lmInfo.appDataSizeBytes / sizeof(quint16);
	}

	int ScriptLmDescription::appDataSizeBytes() const
	{
		return m_lmInfo.appDataSizeBytes;
	}

	int ScriptLmDescription::optoInterfaceDataOffset() const
	{
		return m_lmDesc.optoInterface().m_optoInterfaceDataOffset;
	}

	int ScriptLmDescription::optoPortDataSize() const
	{
		return m_lmDesc.optoInterface().m_optoPortDataSize;
	}
}

