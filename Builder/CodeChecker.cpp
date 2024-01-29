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

	void CodeChecker::MemArea::addSizeW(quint32 sizeW)
	{
		Q_ASSERT(m_sizeW != NOT_INIT);
		m_sizeW += sizeW;
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

	CodeChecker::CodeChecker(const ModuleLogicCompiler& compiler) :
		m_compiler(compiler)
	{
	}

	CodeChecker::~CodeChecker()
	{
		DELETE_ARRAY_IF_NOT_NULL(m_mem);
	}

	bool CodeChecker::check(const AppLogicCode& appLogicCode)
	{
		Q_ASSERT(appLogicCode.codeType() == AppLogicCode::Type::AllCode);

		bool result = init();

		if (result == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("CodeChecker init error!"));
		}

		for(const CodeItem& ci : appLogicCode.code())
		{
			bool res = check(ci);

			if (res == false)
			{
				logError(ci, "check error");
			}

			result &= res;
		}

		return result;
	}

	bool CodeChecker::init()
	{
		m_lmDesc = m_compiler.getLmDescription();

		TEST_PTR_RETURN_FALSE(m_lmDesc);

		m_lmCommands = &m_lmDesc->commands();

		m_log = m_compiler.log();

		TEST_PTR_RETURN_FALSE(m_log);

		m_memSizeW = m_lmDesc->memory().m_appMemorySize;
		m_mem = new quint16 [m_memSizeW];

		std::memset(m_mem, 0, m_memSizeW * sizeof(quint16));

		m_bitMemArea.setStartAddr(m_lmDesc->memory().m_appLogicBitDataOffset);
		m_bitMemArea.setSizeW(m_lmDesc->memory().m_appLogicBitDataSize);

		m_appWordMemArea.setStartAddr(m_lmDesc->memory().m_appLogicWordDataOffset);
		m_appWordMemArea.setSizeW(m_lmDesc->memory().m_appLogicWordDataSize);

		bool result = true;

		result &= initReadableAreas();
		result &= initWritableAreas();
		result &= initPartialWrittenAddresses();
		result &= initLoopbackDiscretes();

		return result;
	}

	bool CodeChecker::initReadableAreas()
	{
		// Input data areas of I/O modules actual installed in chassis
		//
		for(const auto& [place, module] : m_compiler.modules())
		{
			Q_ASSERT(module.place == place);

			if (place > m_lmDesc->memory().m_moduleCount)
			{
				Q_ASSERT(false);
				continue;
			}

			MemArea ma;

			if (place == 0)
			{
				// LM diagnostics data
				//
				ma.setStartAddr(m_lmDesc->memory().m_txDiagDataOffset);
				ma.setSizeW(m_lmDesc->memory().m_txDiagDataSize);

				m_readAreas.insert({ma.startAddr(), ma});
				initToRead(ma);

				// LM module app data
				//
				ma.setStartAddr(module.txAppDataOffset);
				ma.setSizeW(module.txAppDataSize);

				m_readAreas.insert({ma.startAddr(), ma});
				initToRead(ma);
			}
			else
			{
				quint32 moduleDataOffset = m_lmDesc->memory().m_moduleDataOffset +
											(place - 1)  * m_lmDesc->memory().m_moduleDataSize;

				Q_ASSERT(static_cast<quint32>(module.moduleDataOffset) == moduleDataOffset);
				Q_ASSERT(static_cast<quint32>(module.txDataSize) <= m_lmDesc->memory().m_moduleDataSize);

				// append module diag data area
				//
				ma.setStartAddr(moduleDataOffset + module.txDiagDataOffset);
				ma.setSizeW(module.txDiagDataSize);

				m_readAreas.insert({ma.startAddr(), ma});
				initToRead(ma);

				if (module.isOptoModule() == false)
				{
					// for non-opto modules also append App data area
					//
					ma.setStartAddr(moduleDataOffset + module.txAppDataOffset);
					ma.setSizeW(module.txAppDataSize);

					m_readAreas.insert({ma.startAddr(), ma});
					initToRead(ma);
				}
			}
		}

		// Opto ports actual rx data
		//
		std::vector<MemArea> optoRxAreas;

		bool result = m_compiler.getLmAssociatedOptoPortsRxAreas(&optoRxAreas);

		RETURN_IF_FALSE(result);

		for(const MemArea& ma : optoRxAreas)
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
		std::vector<std::pair<quint32, quint32>> framesInfo;

		result = m_compiler.getTuningSignalsFramesInfo(&framesInfo);

		for(auto const& p : framesInfo)
		{
			MemArea tuningFrame;

			tuningFrame.setStartAddr(p.first);
			tuningFrame.setSizeW(p.second);

/*			LOG_MESSAGE(m_log, QString("----- Tuning frame start %1 end %2 sizeW %3").
							arg(tuningFrame.startAddr()).
							arg(tuningFrame.startAddr() + tuningFrame.sizeW() - 1).
							arg(tuningFrame.sizeW())); */

			if (tuningFrame.sizeW() > 0)
			{
				m_readAreas.insert({tuningFrame.startAddr(), tuningFrame});
				initToRead(tuningFrame);
			}
		}

		// Word memory
		//
		MemArea wordMem(m_lmDesc->memory().m_appLogicWordDataOffset,
						m_lmDesc->memory().m_appLogicWordDataSize);

		m_readAreas.insert({wordMem.startAddr(), wordMem});

		joiningSequentialAreas(&m_readAreas);

		return true;
	}

	bool CodeChecker::initWritableAreas()
	{
		// Output data areas of I/O modules actual installed in chassis
		//
		for(const auto& [place, module] : m_compiler.modules())
		{
			Q_ASSERT(module.place == place);

			if (place > m_lmDesc->memory().m_moduleCount)
			{
				Q_ASSERT(false);
				continue;
			}

			MemArea ma;

			if (place == 0)
			{
				ma.setStartAddr(module.txAppDataOffset);		// tx... -  its Ok!
				ma.setSizeW(module.txAppDataSize);

				m_writeAreas.insert({ma.startAddr(), ma});
			}
			else
			{
				if (module.isOptoModule() == false)
				{
					quint32 moduleDataOffset = m_lmDesc->memory().m_moduleDataOffset +
												(place - 1) * m_lmDesc->memory().m_moduleDataSize;

					Q_ASSERT(static_cast<quint32>(module.moduleDataOffset) == moduleDataOffset);
					Q_ASSERT(static_cast<quint32>(module.txDataSize) <= m_lmDesc->memory().m_moduleDataSize);

					ma.setStartAddr(moduleDataOffset + module.rxAppDataOffset);
					ma.setSizeW(module.rxAppDataSize);

					m_writeAreas.insert({ma.startAddr(), ma});
				}
			}

/*			LOG_MESSAGE(m_log, QString("Module %1 write area %2 sizeW %3").
						arg(module.device->equipmentIdTemplate()).
						arg(ma.startAddr()).
						arg(ma.sizeW()));*/
		}

		// Opto ports actual tx data
		//
		std::vector<MemArea> optoTxAreas;

		bool result = m_compiler.getLmAssociatedOptoPortsTxAreas(&optoTxAreas);

		RETURN_IF_FALSE(result);

		for(const MemArea& ma : optoTxAreas)
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

		joiningSequentialAreas(&m_writeAreas);

		return true;
	}

	void CodeChecker::joiningSequentialAreas(std::map<quint32, MemArea>* areas)
	{
		TEST_PTR_RETURN(areas);

		if (areas->empty() == true)
		{
			return;
		}

		std::map<quint32, MemArea> joinedWriteAreas;

		auto it = areas->begin();

		MemArea prevMemArea = it->second;

		it++;

		while(it != areas->end())
		{
			const MemArea& memArea = it->second;

			if (prevMemArea.startAddr() + prevMemArea.sizeW() == memArea.startAddr())
			{
				prevMemArea.addSizeW(memArea.sizeW());
			}
			else
			{
				joinedWriteAreas.insert({ prevMemArea.startAddr(), prevMemArea});
				prevMemArea = memArea;
			}

			it++;
		}

		joinedWriteAreas.insert({ prevMemArea.startAddr(), prevMemArea });

		areas->swap(joinedWriteAreas);
	}

	bool CodeChecker::initPartialWrittenAddresses()
	{
		const LmMemoryMap& mem = m_compiler.lmMemoryMap();

		for(int i = 0; i < LmMemoryMap::BIT_ACCUMULATOR_SIZE_W; i++)
		{
			m_addrCanBeParialWritten.insert(mem.bitAccumulatorAddress() + i);
		}

		m_addrCanBeParialWritten.insert(mem.constBitsAddress());

		if (mem.acquiredDiscreteOutputSignalsSizeW() > 0)
		{
			m_addrCanBeParialWritten.insert(mem.acquiredDiscreteOutputSignalsAddress() +
											mem.acquiredDiscreteOutputSignalsSizeW() - 1);
		}

		if (mem.acquiredDiscreteInternalSignalsSizeW() > 0)
		{
			m_addrCanBeParialWritten.insert(mem.acquiredDiscreteInternalSignalsAddress() +
											mem.acquiredDiscreteInternalSignalsSizeW() - 1);
		}

		return true;
	}

	bool CodeChecker::initLoopbackDiscretes()
	{
		bool result = true;

		QList<const UalSignal*> loopbackSignals = m_compiler.getLoopbacksUalSignals();

		for(const UalSignal* s : loopbackSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == true ||
				s->isDiscrete() == false)
			{
				continue;
			}

			if (s->ualAddrIsValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (m_compiler.lmMemoryMap().addressInBitMemory(s->ualAddr().offset()) == true)
			{
				writeBit(s->ualAddr().offset(), s->ualAddr().bit());
			}
		}

		return result;
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

		LOG_INTERNAL_ERROR_MSG(m_log, QString("%1, command: %2").arg(err).arg(cmd.getAsmCode(m_lmDesc, false, false)));
	}

	bool CodeChecker::check(const CodeItem& cmd)
	{
		if (cmd.isCommand() == false)
		{
			return true;
		}

		TEST_PTR_RETURN_FALSE(m_lmCommands);

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
		LmCommandCode cmdCode = cmd.lmCommandCode();

		auto it = m_lmCommands->find(cmdCode);

		if (it == m_lmCommands->end())
		{
			logError(cmd, QString("Unknown command code %1").arg(cmdCode));
			return false;
		}

		auto it2 = m_checkFuncMap.find(it->second.checkFunc);

		if (it2 == m_checkFuncMap.end())
		{
			logError(cmd, QString("Unknown command check function '%1'").arg(it->second.checkFunc));
			return false;
		}

		CheckFuncPtr checkFuncPtr = it2->second;

		return (this->*checkFuncPtr)(cmd);
	}

	bool CodeChecker::check_nothing(const CodeItem& cmd)
	{
		Q_UNUSED(cmd);
		return true;
	}

	bool CodeChecker::check_startafb(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_mov(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 writeAddr = cmd.getWord2();

		return checkCanRead16(cmd, readAddr) &&
			   checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::check_mov_addr_acc(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();

		return checkCanWrite16(cmd, writeAddr) &&
			   addrNotInBitMemArea(writeAddr, 1);
	}

	bool CodeChecker::check_mov_acc_addr(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord2();

		return checkCanRead16(cmd, readAddr);
	}

	bool CodeChecker::check_movmem(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 writeAddr = cmd.getWord2();
		quint32 n = cmd.getWord4();

		return checkCanRead(cmd, readAddr, n) &&
			   checkCanWrite(cmd, writeAddr, n) &&
			   addrNotInBitMemArea(writeAddr, n);
	}

	bool CodeChecker::check_movc(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();

		return checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::check_movbc(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();
		quint32 bitNo = cmd.getWord4();

		return checkCanWriteBit(cmd, writeAddr, bitNo);
	}

	bool CodeChecker::check_wrfb(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();

		return  checkCanRead16(cmd, readAddr) &&
				checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_rdfb(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord3();

		return  checkFbTypeAndInstance(cmd) &&
				checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::check_wrfbc(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_wrfbb(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 bitNo = cmd.getWord4();

		return  checkCanReadBit(cmd, readAddr, bitNo) &&
				checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_rdfbb(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord3();
		quint32 bitNo = cmd.getWord4();

		return  checkFbTypeAndInstance(cmd) &&
				checkCanWriteBit(cmd, writeAddr,  bitNo) &&
				addrInBitMemArea(writeAddr, 1);
	}

	bool CodeChecker::check_rdfbcmp(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_setmem(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();
		quint32 sizeW = cmd.getWord4();

		return checkCanWrite(cmd, writeAddr, sizeW) &&
			   addrNotInBitMemArea(writeAddr, sizeW);
	}

	bool CodeChecker::check_movb(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 readBitNo = cmd.getBitNo1();

		quint32 writeAddr = cmd.getWord2();
		quint32 writeBitNo = cmd.getBitNo2();

		return  checkCanReadBit(cmd, readAddr, readBitNo) &&
				checkCanWriteBit(cmd, writeAddr, writeBitNo);
	}

	bool CodeChecker::check_movb_acc_addr(const CodeItem& cmd)
	{
		Address16 readAddr = cmd.srcBitAddr();

		return  checkCanReadBit(cmd, readAddr.offset(), readAddr.bit());
	}

	bool CodeChecker::check_movb_addr_acc(const CodeItem& cmd)
	{
		Address16 writeAddr = cmd.destBitAddr();

		return  checkCanWriteBit(cmd, writeAddr.offset(), writeAddr.bit());
	}

	bool CodeChecker::check_nstart(const CodeItem& cmd)
	{
		checkFbTypeAndInstance(cmd);
		return true;
	}

	bool CodeChecker::check_mov32(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 writeAddr = cmd.getWord2();

		return checkCanRead32(cmd, readAddr) &&
			   checkCanWrite32(cmd, writeAddr) &&
			   addrNotInBitMemArea(writeAddr, 2);
	}

	bool CodeChecker::check_movc32(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();

		return checkCanWrite32(cmd, writeAddr) &&
			   addrNotInBitMemArea(writeAddr, 2);
	}

	bool CodeChecker::check_wrfb32(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();

		return  checkCanRead32(cmd, readAddr) &&
				checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_rdfb32(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord3();

		return  checkFbTypeAndInstance(cmd) &&
				checkCanWrite32(cmd, writeAddr);
	}

	bool CodeChecker::check_wrfbc32(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_rdfbcmp32(const CodeItem& cmd)
	{
		return checkFbTypeAndInstance(cmd);
	}

	bool CodeChecker::check_movcmpf(const CodeItem& cmd)
	{
		quint32 writeAddr = cmd.getWord2();
		quint32 bitNo = cmd.getWord3();

		return checkCanWriteBit(cmd, writeAddr, bitNo);
	}

	bool CodeChecker::check_pmov(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 writeAddr = cmd.getWord2();

		return checkCanRead(cmd, readAddr, 1, true) &&
			   checkCanWrite16(cmd, writeAddr);
	}

	bool CodeChecker::check_pmov32(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 writeAddr = cmd.getWord2();

		return checkCanRead(cmd, readAddr, 2, true) &&
			   checkCanWrite32(cmd, writeAddr);
	}

	bool CodeChecker::check_fillb(const CodeItem& cmd)
	{
		quint32 readAddr = cmd.getWord3();
		quint32 readBitNo = cmd.getWord4();

		quint32 writeAddr = cmd.getWord2();

		return  checkCanReadBit(cmd, readAddr, readBitNo) &&
				checkCanWrite16(cmd, writeAddr);
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

	bool CodeChecker::checkCanRead(const CodeItem& cmd, quint32 readAddr, quint32 sizeW, bool enableReadUnwritten) const
	{
		const MemArea& areaToRead = findMemAreaToRead(readAddr, sizeW);

		if (areaToRead.isValid() == false)
		{
			logError(cmd, QString("Can't read address %1").arg(readAddr));
			return false;
		}

		for(quint32 i = 0; i < sizeW; i++)
		{
			if (readAddr + i >= m_memSizeW)
			{
				Q_ASSERT(false);
				logError(cmd, QString("Read address %1 out of range").arg(readAddr + i));
				return false;
			}
			else
			{
				if (enableReadUnwritten == false)
				{
					quint16 memValue = m_mem[readAddr + i];

					if (memValue != 0xFFFF)
					{
						if (addrCanBePartialWritten(readAddr + i) == false || memValue == 0)
						{
							logError(cmd, QString("Unwritten memory read on address %1").arg(readAddr + i));
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	bool CodeChecker::checkCanReadBit(const CodeItem& cmd, quint32 readAddr, quint32 bitNo) const
	{
		const MemArea& areaToRead = findMemAreaToRead(readAddr, 1);

		if (areaToRead.isValid() == false)
		{
			logError(cmd, QString("Can't read address %1[%2]").arg(readAddr).arg(bitNo));
			return false;
		}

		if (bitNo > 15)
		{
			logError(cmd, QString("Can't read address %1[%2], bitNo out ouf range").arg(readAddr).arg(bitNo));
			return false;
		}

		quint16 result = m_mem[readAddr] & (0x0001 << bitNo);

		if (result == 0)
		{
			logError(cmd, QString("Unwritten memory read on address %1[%2]").arg(readAddr).arg(bitNo));
			return false;
		}

		return true;
	}

	bool CodeChecker::addrCanBePartialWritten(quint32 readAddr) const
	{
		return (m_addrCanBeParialWritten.find(readAddr) != m_addrCanBeParialWritten.end());
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

		for(quint32 i = 0; i < sizeW; i++)
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

	bool CodeChecker::checkCanWriteBit(const CodeItem& cmd, quint32 writeAddr, quint32 bitNo) const
	{
		if (m_bitMemArea.addressInArea(writeAddr, 1) == false &&
			m_appWordMemArea.addressInArea(writeAddr, 1) == false)
		{
			logError(cmd, QString("Can't write address %1[%2]").arg(writeAddr).arg(bitNo));
			return false;
		}

		const MemArea& areaToWrite = findMemAreaToWrite(writeAddr, 1);

		if (areaToWrite.isValid() == false)
		{
			logError(cmd, QString("Can't write address %1[%2]").arg(writeAddr).arg(bitNo));
			return false;
		}

		if (bitNo > 15)
		{
			logError(cmd, QString("Can't write address %1[%2], bitNo out ouf range").arg(writeAddr).arg(bitNo));
			return false;
		}

		writeBit(writeAddr, bitNo);

		return true;
	}

	void CodeChecker::writeBit(quint32 writeAddr, quint32 bitNo) const
	{
		TEST_PTR_RETURN(m_mem);
		Q_ASSERT(writeAddr < m_memSizeW);
		Q_ASSERT(bitNo < 16);

		m_mem[writeAddr] |= (0x0001 << bitNo);
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
			it = std::prev(it);
			result = it->second.addressInArea(addr, sizeW);
		}

		if (result == false)
		{
			return m_notValidArea;
		}

		return it->second;
	}

	bool CodeChecker::addrInBitMemArea(quint32 addr, quint32 sizeW) const
	{
		return m_bitMemArea.addressInArea(addr, sizeW);
	}

	bool CodeChecker::addrNotInBitMemArea(quint32 addr, quint32 sizeW) const
	{
		return m_bitMemArea.addressInArea(addr, 1) == false &&
			   m_bitMemArea.addressInArea(addr + sizeW - 1, 1) == false;
	}

}
