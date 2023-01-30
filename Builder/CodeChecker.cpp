#include "CodeChecker.h"
#include "ModuleLogicCompiler.h"

namespace Builder
{
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
/*		// I/O modules input data
		//
		for(quint32 i = 0; i < m_lmDesc.memory().m_moduleCount; i++)
		{
			MemArea ma(m_lmDesc.memory().m_moduleDataOffset + i * m_lmDesc.memory().m_moduleDataSize,
					   m_lmDesc.memory().m_moduleDataSize);

			m_readAreas.push_back(ma);

			initToRead(ma);
		}

		// Opto interface input data
		//
		for(quint32 i = 0; i < m_lmDesc.optoInterface().m_optoPortCount; i++)
		{
			MemArea ma(m_lmDesc.optoInterface().m_optoInterfaceDataOffset +
							i * (m_lmDesc.optoInterface().m_optoPortAppDataOffset +
								 m_lmDesc.optoInterface().m_optoPortAppDataSize),
					   m_lmDesc.optoInterface().m_optoPortAppDataSize);

			m_readAreas.push_back(ma);

			initToRead(ma);
		}

		// Bit memory
		//
		MemArea bitMem(m_lmDesc.memory().m_appLogicBitDataOffset,
					   m_lmDesc.memory().m_appLogicBitDataSize);

		m_readAreas.push_back(bitMem);

		// Tuning memory
		//
		MemArea tuningMem(m_lmDesc.memory().m_tuningDataOffset,
						  m_lmDesc.memory().m_tuningDataSize);

		m_readAreas.push_back(tuningMem);
		initToRead(tuningMem);

		// Word memory
		//
		MemArea wordMem(m_lmDesc.memory().m_appLogicWordDataOffset,
						m_lmDesc.memory().m_appLogicWordDataSize);

		m_readAreas.push_back(wordMem);


*/
		return true;
	}

	void CodeChecker::initToRead(const MemArea& ma)
	{
		if (ma.isValid() == false)
		{
			Q_ASSERT(false);
			return;
		}

		if (ma.startAddr >= m_memSizeW ||
			(ma.startAddr + ma.sizeW - 1) > m_memSizeW)
		{
			Q_ASSERT(false);
			return;
		}

		std::memset(m_mem + ma.startAddr, 0xFF, ma.sizeW * sizeof(quint16));
	}

	bool CodeChecker::initWritableAreas()
	{
		return true;
	}

	void CodeChecker::logError(const QString& err, const CodeItem& cmd)
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
			logError("Command address out of range", cmd);
			return false;
		}

		// Command parameters checking
		//
		int funcIndex = static_cast<int>(cmd.getOpcode());

		if (funcIndex < 0 && funcIndex >= LM_COMMANDS_COUNT)
		{
			logError("Invalid command OpCode", cmd);
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
		// check addresses
		//
		int readAddr = cmd.getWord3();
		int writeAddr = cmd.getWord2();

		return checkCanRead(cmd, readAddr, 1) &&
			   checkCanWrite(cmd, writeAddr, 1);
	}

	bool CodeChecker::checkMovMem(const CodeItem& cmd)
	{
		/*if (addressInBitMemory(addrTo) ||
			addressInBitMemory(addrTo + sizeW - 1))
		{
			// Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.
			//
			m_log->errALC5066(addrTo, addrFrom, sizeW);
			m_result = false;
		}

		readArea(addrFrom, sizeW);
		writeArea(addrTo, sizeW);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMovConst(const CodeItem& cmd)
	{
		//write16(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMovBitConst(const CodeItem& cmd)
	{
		/*
		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{

			//	Command 'MOVBC %1, %2, #%3' can't write out of application bit- or word-addressed memory.
			//
			m_log->errALC5067(addrTo, bitNo, constBit);

			m_result = false;
		}

		write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkWriteFuncBlock(const CodeItem& cmd)
	{
		//	read16(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkReadFuncBlock(const CodeItem& cmd)
	{
		// write16(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkWriteFuncBlockConst(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkWriteFuncBlockBit(const CodeItem& cmd)
	{
		// read16(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkReadFuncBlockBit(const CodeItem& cmd)
	{
		/*
		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{
			Q_ASSERT(false);			// RDFBB command can write only in bit- or word-addressed memory
			m_result = false;
			return;
		}

		m_memoryMap->write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkReadFuncBlockTest(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkSetMem(const CodeItem& cmd)
	{
		/*
		if (addressInBitMemory(addr) ||
			addressInBitMemory(addr + sizeW - 1))
		{
			Q_ASSERT(false);			// SETMEM command can't write to bit-addressed memory
			m_result = false;
		}

		writeArea(addr, sizeW);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMovBit(const CodeItem& cmd)
	{
		/*if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{

			// Command 'MOVB %1[%2], %3[%4]' can't write out of application bit- or word-addressed memory.
			//
			m_log->errALC5089(addrTo, bitTo, addrFrom, bitFrom);

			m_result = false;
		}

		//

		read16(addrFrom);
		write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkNstart(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkAppStart(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMov32(const CodeItem& cmd)
	{
		/*read32(addrFrom);
		write32(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMovConst32(const CodeItem& cmd)
	{
		// write32(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkWriteFuncBlock32(const CodeItem& cmd)
	{
		// read32(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkReadFuncBlock32(const CodeItem& cmd)
	{
		// write32(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkWriteFuncBlockConst32(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkReadFuncBlockTest32(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkMovConstIfFlag(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkPrevMov(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkPrevMov32(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkFill(const CodeItem& cmd)
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeChecker::checkFbTypeAndInstance(const CodeItem& cmd)
	{
		int fbType = cmd.getFbType();

		std::shared_ptr<Afb::AfbComponent> afb = m_lmDesc->component(fbType);

		if (afb == nullptr)
		{
			logError("Unknown AFB OpCode", cmd);
			return false;
		}

		int fbInstance = cmd.getFbInstance();

		if (fbInstance >= afb->maxInstCount())
		{
			logError("AFB instance exceeds max instance count", cmd);
			return false;
		}

		return true;
	}

	bool CodeChecker::checkCanRead(const CodeItem& cmd, quint32 readAddr, quint32 sizeW)
	{
		return true;
	}

	bool CodeChecker::checkCanWrite(const CodeItem& cmd, quint32 writeAddr, quint32 sizeW)
	{
		return true;
	}
}
