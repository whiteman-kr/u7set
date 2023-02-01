#include "CodeChecker.h"
#include "ModuleLogicCompiler.h"

namespace Builder
{

	// ----------------------------------------------------------------------------------
	//
	// CodeChecker::MemArea class implementation
	//
	// ----------------------------------------------------------------------------------

	CodeChecker::MemArea::MemArea()
	{
	}

	CodeChecker::MemArea::MemArea(quint32 startAddr, quint32 sizeW) :
		m_startAddr(startAddr),
		m_sizeW(sizeW)
	{
	}

	bool CodeChecker::MemArea::isValid() const
	{
		return (m_startAddr != NOT_INIT && m_sizeW != NOT_INIT);
	}

	void CodeChecker::MemArea::setStartAddr(quint32 startAddr)
	{
		m_startAddr = startAddr;
	}

	quint32 CodeChecker::MemArea::startAddr() const
	{
		Q_ASSERT(m_startAddr != NOT_INIT);
		return m_startAddr;
	}

	void CodeChecker::MemArea::setSizeW(quint32 sizeW)
	{
		m_sizeW = sizeW;
	}

	quint32 CodeChecker::MemArea::sizeW() const
	{
		Q_ASSERT(m_sizeW != NOT_INIT);
		return m_sizeW;
	}

	bool CodeChecker::MemArea::addressInArea(quint32 addr, quint32 sizeW) const
	{
		return addr >= m_startAddr &&
			   (addr + sizeW) <= (m_startAddr + m_sizeW);
	}

	// ----------------------------------------------------------------------------------
	//
	// CodeChecker class implementation
	//
	// ----------------------------------------------------------------------------------

	CheckFuncPtr CodeChecker::m_checkFunc[LM_COMMANDS_COUNT] =
	{
		&CodeChecker::checkNoCommand,
		&CodeChecker::checkNop,
		&CodeChecker::checkStart,
		&CodeChecker::checkStop,
		&CodeChecker::checkMov,
		&CodeChecker::checkMovMem,
		&CodeChecker::checkMovConst,
		&CodeChecker::checkMovBitConst,
		&CodeChecker::checkWriteFuncBlock,
		&CodeChecker::checkReadFuncBlock,
		&CodeChecker::checkWriteFuncBlockConst,
		&CodeChecker::checkWriteFuncBlockBit,
		&CodeChecker::checkReadFuncBlockBit,
		&CodeChecker::checkReadFuncBlockTest,
		&CodeChecker::checkSetMem,
		&CodeChecker::checkMovBit,
		&CodeChecker::checkNstart,
		&CodeChecker::checkAppStart,
		&CodeChecker::checkMov32,
		&CodeChecker::checkMovConst32,
		&CodeChecker::checkWriteFuncBlock32,
		&CodeChecker::checkReadFuncBlock32,
		&CodeChecker::checkWriteFuncBlockConst32,
		&CodeChecker::checkReadFuncBlockTest32,
		&CodeChecker::checkMovConstIfFlag,
		&CodeChecker::checkPrevMov,
		&CodeChecker::checkPrevMov32,
		&CodeChecker::checkFill
	};

	CodeChecker::CodeChecker(const ModuleLogicCompiler& compiler) :
		m_compiler(compiler)
	{
	}

	CodeChecker::~CodeChecker()
	{
		DELETE_IF_NOT_NULL(m_mem);
	}

	bool CodeChecker::check(const AppLogicCode& appLogicCode)
	{
		Q_ASSERT(appLogicCode.codeType() == AppLogicCode::Type::AllCode);

		bool result = init();

		RETURN_IF_FALSE(result);

		for(const CodeItem& ci : appLogicCode.code())
		{
			result &= check(ci);
		}

		return result;
	}

	bool CodeChecker::init()
	{
		auto lmDesc = m_compiler.getLmDescription();

		TEST_PTR_RETURN_FALSE(lmDesc);

		m_lmDesc = lmDesc.get();

		m_log = m_compiler.log();

		TEST_PTR_RETURN_FALSE(m_log);

		m_memSizeW = lmDesc->memory().m_appMemorySize;
		m_mem = new quint16 [m_memSizeW];

		std::memset(m_mem, 0, m_memSizeW * sizeof(quint16));

		bool result = true;

		result &= initReadableAreas();
		result &= initWritableAreas();

		return result;
	}

	bool CodeChecker::initReadableAreas()
	{
		// Input data areas of I/O modules actual installed in chassis
		//
		for(const ModuleLogicCompiler::Module& module : m_compiler.modules())
		{
			quint32 place = module.place;

			if (place > m_lmDesc->memory().m_moduleCount)
			{
				Q_ASSERT(false);
				continue;
			}

			MemArea ma;

			if (place == 0)
			{
				// LM module
				//
				ma.setStartAddr(module.txAppDataOffset);
				ma.setSizeW(module.txAppDataSize);
			}
			else
			{
				quint32 moduleDataOffset = m_lmDesc->memory().m_moduleDataOffset +
											(place - 1)  * m_lmDesc->memory().m_moduleDataSize;

				Q_ASSERT(static_cast<quint32>(module.moduleDataOffset) == moduleDataOffset);
				Q_ASSERT(static_cast<quint32>(module.txDataSize) <= m_lmDesc->memory().m_moduleDataSize);

				ma.setStartAddr(module.moduleDataOffset);
				ma.setSizeW(module.txDataSize);
			}

			m_readAreas.insert({ma.startAddr(), ma});
			initToRead(ma);
		}

		// LM's opto interface actual rx data
		//
		std::vector<MemArea> lmOptoRxAreas;

		bool result = m_compiler.getLmOptoPortsRxAreas(&lmOptoRxAreas);

		RETURN_IF_FALSE(result);

		for(const MemArea& ma : lmOptoRxAreas)
		{
			m_readAreas.insert({ma.startAddr(), ma});
			initToRead(ma);
		}

		// Bit memory
		//
		MemArea bitMem(m_lmDesc->memory().m_appLogicBitDataOffset,
					   m_lmDesc->memory().m_appLogicBitDataSize);

		m_readAreas.insert({bitMem.startAddr(), bitMem});

		// Actual used tuning memory
		//
		MemArea usedTuningMem;

		result = m_compiler.getLmUsedTuningArea(&usedTuningMem);

		RETURN_IF_FALSE(result);

		if (usedTuningMem.sizeW() > 0)
		{
			m_readAreas.insert({usedTuningMem.startAddr(), usedTuningMem});
			initToRead(usedTuningMem);
		}

		// Word memory
		//
		MemArea wordMem(m_lmDesc->memory().m_appLogicWordDataOffset,
						m_lmDesc->memory().m_appLogicWordDataSize);

		m_readAreas.insert({wordMem.startAddr(), wordMem});

		// LM diagnostics data
		//
		MemArea lmDiagData(m_lmDesc->memory().m_txDiagDataOffset,
						   m_lmDesc->memory().m_txDiagDataSize);

		m_readAreas.insert({lmDiagData.startAddr(), lmDiagData});
		initToRead(lmDiagData);

		return true;
	}

	bool CodeChecker::initWritableAreas()
	{
		// Output data areas of I/O modules actual installed in chassis
		//
		for(const ModuleLogicCompiler::Module& module : m_compiler.modules())
		{
			quint32 place = module.place;

			if (place > m_lmDesc->memory().m_moduleCount)
			{
				Q_ASSERT(false);
				continue;
			}

			MemArea ma;

			if (place == 0)
			{
				ma.setStartAddr(module.txAppDataOffset);		// ! its Ok
				ma.setSizeW(module.txAppDataSize);
			}
			else
			{
				quint32 moduleDataOffset = m_lmDesc->memory().m_moduleDataOffset +
											(place - 1) * m_lmDesc->memory().m_moduleDataSize;

				Q_ASSERT(static_cast<quint32>(module.moduleDataOffset) == moduleDataOffset);
				Q_ASSERT(static_cast<quint32>(module.txDataSize) <= m_lmDesc->memory().m_moduleDataSize);

				ma.setStartAddr(module.moduleDataOffset + module.rxAppDataOffset);
				ma.setSizeW(module.rxAppDataSize);
			}

			m_writeAreas.insert({ma.startAddr(), ma});
		}

		// LM's opto interface actual tx data
		//
		std::vector<MemArea> lmOptoTxAreas;

		bool result = m_compiler.getLmOptoPortsTxAreas(&lmOptoTxAreas);

		RETURN_IF_FALSE(result);

		for(const MemArea& ma : lmOptoTxAreas)
		{
			m_writeAreas.insert({ma.startAddr(), ma});
		}

		// Bit memory
		//
		MemArea bitMem(m_lmDesc->memory().m_appLogicBitDataOffset,
					   m_lmDesc->memory().m_appLogicBitDataSize);

		m_writeAreas.insert({bitMem.startAddr(), bitMem});

		// Word memory
		//
		MemArea wordMem(m_lmDesc->memory().m_appLogicWordDataOffset,
						m_lmDesc->memory().m_appLogicWordDataSize);

		m_writeAreas.insert({wordMem.startAddr(), wordMem});

		return true;
	}

	void CodeChecker::initToRead(const MemArea& ma)
	{
		if (ma.isValid() == false)
		{
			Q_ASSERT(false);
			return;
		}

		if (ma.startAddr() >= m_memSizeW ||
			(ma.startAddr() + ma.sizeW() - 1) > m_memSizeW)
		{
			Q_ASSERT(false);
			return;
		}

		std::memset(m_mem + ma.startAddr(), 0xFF, ma.sizeW() * sizeof(quint16));
	}

	void CodeChecker::logError(const CodeItem& cmd, const QString& err) const
	{
		TEST_PTR_RETURN(m_log);

		LOG_INTERNAL_ERROR_MSG(m_log, QString("%1, command: %2").arg(err).arg(cmd.getAsmCode(false)));
	}

	bool CodeChecker::check(const CodeItem& cmd)
	{
		if (cmd.isCommand() == false)
		{
			return true;
		}

		// Command address checking
		//
		bool res =	cmd.address() >= 0 &&
					cmd.address() < static_cast<int>(m_lmDesc->memory().m_codeMemorySize);

		if (res == false)
		{
			logError(cmd, "Command address out of range");
			return false;
		}

		// Command parameters checking
		//
		int funcIndex = static_cast<int>(cmd.getOpcode());

		if (funcIndex < 0 && funcIndex >= LM_COMMANDS_COUNT)
		{
			logError(cmd, "Invalid command OpCode");
			return false;
		}

		CheckFuncPtr checkFuncPtr = m_checkFunc[funcIndex];

		return (this->*checkFuncPtr)(cmd);
	}

	bool CodeChecker::checkNoCommand(const CodeItem& cmd)
	{
		Q_UNUSED(cmd);
		return true;
	}

	bool CodeChecker::checkNop(const CodeItem& cmd)
	{
		Q_UNUSED(cmd);
		return true;
	}

	bool CodeChecker::checkStart(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::checkStop(const CodeItem& cmd)
	{
		Q_UNUSED(cmd);
		return true;
	}

	bool CodeChecker::checkMov(const CodeItem& cmd)
	{
		int readAddr = cmd.getWord3();
		int writeAddr = cmd.getWord2();

		return checkCanRead16(cmd, readAddr) &&
			   checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::checkMovMem(const CodeItem& cmd)
	{
		int readAddr = cmd.getWord3();
		int writeAddr = cmd.getWord2();
		int n = cmd.getWord4();

		return checkCanRead(cmd, readAddr, n) &&
			   checkCanWrite(cmd, writeAddr, n);
	}

	bool CodeChecker::checkMovConst(const CodeItem& cmd)
	{
		int writeAddr = cmd.getWord2();

		return checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::checkMovBitConst(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkWriteFuncBlock(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkReadFuncBlock(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkWriteFuncBlockConst(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkWriteFuncBlockBit(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkReadFuncBlockBit(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkReadFuncBlockTest(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkSetMem(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkMovBit(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkNstart(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkAppStart(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkMov32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkMovConst32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkWriteFuncBlock32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkReadFuncBlock32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkWriteFuncBlockConst32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkReadFuncBlockTest32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkMovConstIfFlag(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkPrevMov(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkPrevMov32(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkFill(const CodeItem& cmd)
	{
		return true;
	}

	bool CodeChecker::checkFbTypeAndInstance(const CodeItem& cmd)
	{
		int fbType = cmd.getFbType();

		std::shared_ptr<Afb::AfbComponent> afb = m_lmDesc->component(fbType);

		if (afb == nullptr)
		{
			logError(cmd, "Unknown AFB OpCode");
			return false;
		}

		int fbInstance = cmd.getFbInstance();

		if (fbInstance >= afb->maxInstCount())
		{
			logError(cmd, "AFB instance exceeds max instance count");
			return false;
		}

		return true;
	}

	bool CodeChecker::checkCanRead16(const CodeItem& cmd, quint32 readAddr) const
	{
		return checkCanRead(cmd, readAddr, 1);
	}

	bool CodeChecker::checkCanRead32(const CodeItem& cmd, quint32 readAddr) const
	{
		return checkCanRead(cmd, readAddr, 2);
	}

	bool CodeChecker::checkCanRead(const CodeItem& cmd, quint32 readAddr, quint32 sizeW) const
	{
		const MemArea& areaToRead = findMemAreaToRead(readAddr, sizeW);

		if (areaToRead.isValid() == false)
		{
			logError(cmd, QString("Can't read address %1").arg(readAddr));
			return false;
		}

		for(int i = 0; i < sizeW; i++)
		{
			if (readAddr + i >= m_memSizeW)
			{
				Q_ASSERT(false);
				logError(cmd, QString("Read address %1 out of range").arg(readAddr + i));
				return false;
			}
			else
			{
				if (m_mem[readAddr + i] != 0xFFFF)
				{
					logError(cmd, QString("Unwritten memory read on address %1").arg(readAddr + i));
					return false;
				}
			}
		}

		return true;
	}

	bool CodeChecker::checkCanWrite16(const CodeItem& cmd, quint32 writeAddr) const
	{
		return checkCanWrite(cmd, writeAddr, 1);
	}

	bool CodeChecker::checkCanWrite32(const CodeItem& cmd, quint32 writeAddr) const
	{
		return checkCanWrite(cmd, writeAddr, 2);
	}

	bool CodeChecker::checkCanWrite(const CodeItem& cmd, quint32 writeAddr, quint32 sizeW) const
	{
		const MemArea& areaToWrite = findMemAreaToWrite(writeAddr, sizeW);

		if (areaToWrite.isValid() == false)
		{
			logError(cmd, QString("Can't write address %1").arg(writeAddr));
			return false;
		}

		for(int i = 0; i < sizeW; i++)
		{
			if (writeAddr + i >= m_memSizeW)
			{
				Q_ASSERT(false);
				logError(cmd, QString("Write address %1 out of range").arg(writeAddr + i));
			}
			else
			{
				m_mem[writeAddr + i] = 0xFFFF;
			}
		}

		return true;
	}

	const CodeChecker::MemArea& CodeChecker::findMemAreaToWrite(quint32 writeAddr, quint32 sizeW) const
	{
		return findMemArea(m_writeAreas, writeAddr, sizeW);
	}

	const CodeChecker::MemArea& CodeChecker::findMemAreaToRead(quint32 readAddr, quint32 sizeW) const
	{
		return findMemArea(m_readAreas, readAddr, sizeW);
	}

	const CodeChecker::MemArea& CodeChecker::findMemArea(const std::map<quint32, MemArea>& areas,
							   quint32 addr, quint32 sizeW) const
	{
		auto it = areas.upper_bound(addr);

		bool result = true;

		if (it == areas.begin())
		{
			result = false;
		}
		else
		{
			it--;

			if (it == areas.end())
			{
				result = false;
			}
			else
			{
				result = it->second.addressInArea(addr, sizeW);
			}
		}

		if (result == false)
		{
			return m_notValidArea;
		}

		return it->second;
	}
}
