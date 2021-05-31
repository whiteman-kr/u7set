#pragma once

#include "../lib/LmDescription.h"

#include "SimLogicModule.h"
#include "SimScriptRamAddress.h"

namespace Sim
{
	class ScriptLmDescription
	{
		Q_GADGET

		Q_PROPERTY(bool isNull READ isNull)
		Q_PROPERTY(RamAddress appDataStartAddr READ appDataStartAddr)

		Q_PROPERTY(int optoInterfaceDataOffset READ optoInterfaceDataOffset)
		Q_PROPERTY(int optoPortDataSize READ optoPortDataSize)

	public:
		ScriptLmDescription() = default;
		~ScriptLmDescription() = default;

		void setLogicModule(std::shared_ptr<LogicModule> lm);

		bool isNull() const;

	public slots:

		bool isAddrInIoModuleBuf(int modulePlace, RamAddress addr) const;
		bool isAddrInAcquiredAppDataBuf(RamAddress addr) const;
		bool isAddrAfterAcquiredAppDataBuf(RamAddress addr) const;

		RamAddress ioModuleBufStartAddr(int modulePlace) const;
		int ioModuleBufSize() const;

		RamAddress appDataStartAddr() const;
		int appDataSizeW() const;
		int appDataSizeBytes() const;

		int optoInterfaceDataOffset() const;
		int optoPortDataSize() const;

	private:
		LmDescription m_lmDesc;
		::LogicModuleInfo m_lmInfo;
	};
}

Q_DECLARE_METATYPE(Sim::ScriptLmDescription);
