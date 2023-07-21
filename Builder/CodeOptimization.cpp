#include "CodeOptimization.h"
#include "ModuleLogicCompiler.h"
#include "../HardwareLib/LmDescription.h"

namespace Builder
{

	QString OptimizationInfo::typeStr(CodeOptimizationType t)
	{
		switch(t)
		{
		case CodeOptimizationType::None:
			Q_ASSERT(false);
			return QStringLiteral("None");

		case CodeOptimizationType::SequentialMoves:
			return QStringLiteral("SequentialMoves");

		case CodeOptimizationType::SequentialConstMoves:
			return QStringLiteral("SequentialConstMoves");

		case CodeOptimizationType::SequentialBitMoves:
			return QStringLiteral("SequentialBitMoves");

		case CodeOptimizationType::BitFilling:
			return QStringLiteral("BitFilling");

		case CodeOptimizationType::BitAccNot:
			return QStringLiteral("BitAccNot");

		case CodeOptimizationType::SequentialAccBitMoves:
			return QStringLiteral("SequentialAccBitMoves");

		case CodeOptimizationType::BitAccAnd:
			return QStringLiteral("BitAccAnd");

		case CodeOptimizationType::BitAccOr:
			return QStringLiteral("BitAccOr");

		default:
			Q_ASSERT(false);
		}

		return QStringLiteral("");
	}

	// ---------------------------------------------------------------------------------------
	//
	// SequenceOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	SequenceOptimization::SequenceOptimization(CodeOptimizationType optimizationType,
											   ModuleLogicCompiler& compiler,
											   CodeSnippet& srcCode) :
		m_optimizationType(optimizationType),
		m_compiler(compiler),
		m_srcCode(srcCode)
	{
	}

	bool SequenceOptimization::optimize()
	{
		if (isOptimizationPossible() == false)
		{
			return true;
		}

		int commandsInSequence = 0;

		CodeSnippetConstIterator firstSequenceCmd;
		CodeSnippetConstIterator lastSequenceCmd;
		CodeSnippetConstIterator it;

		CodeSnippet optiCode;

		optiCode.reserve(static_cast<int>(m_srcCode.itemsCount() * 1.2));

		auto sequenceReinitVars =	[&]() -> void
							{
								commandsInSequence = 0;

								firstSequenceCmd = m_srcCode.end();
								lastSequenceCmd = m_srcCode.end();

								reinitVars();
							};

		auto finalizeSequence =	[&]() -> void
							{
								if (commandsInSequence > 0)
								{
									if (commandsInSequence > 1)
									{
										CodeSnippet replacementCode;

										if (canOptimize() == true &&
											getReplacementCode(replacementCode) == true)
										{
											m_compiler.optimizeCode(m_optimizationType,
																	m_srcCode,
																	firstSequenceCmd,
																	lastSequenceCmd,
																	optiCode,
																	replacementCode);
											lastSequenceCmd++;

											while(lastSequenceCmd != it &&
												  lastSequenceCmd != m_srcCode.end())
											{
												optiCode.append(*lastSequenceCmd);
												lastSequenceCmd++;
											}
										}
										else
										{
											// no optimization possible
											//
											while(firstSequenceCmd != it &&
												  firstSequenceCmd != m_srcCode.end())
											{
												optiCode.append(*firstSequenceCmd);
												firstSequenceCmd++;
											}
										}
									}
									else
									{
										// commandsInSequence == 1
										//
										do
										{
											optiCode.append(*firstSequenceCmd);
											firstSequenceCmd++;

											if (firstSequenceCmd == it)
											{
												break;
											}
										}
										while(true);
									}

									sequenceReinitVars();
								}
							};

		auto inSequence =	[&]() -> bool
							{
								return firstSequenceCmd != m_srcCode.end();
							};

		sequenceReinitVars();

		for(it = m_srcCode.begin(); it != m_srcCode.end(); it++)
		{
			const CodeItem& cmd = *it;

			if (cmd.isComment() == true)
			{
				if (inSequence() == true)
				{
					// continue sequence
					//
				}
				else
				{
					// not in sequence - copy comment to optimized code
					//
					optiCode.append(cmd);
				}

				continue;
			}

			// Commands processing

			if (inSequence() == true)
			{
				if (isSequenceContinue(cmd) == true)
				{
					// continue sequence
					//
					commandsInSequence++;
					lastSequenceCmd = it;
					continue;
				}
				else
				{
					finalizeSequence();
				}
			}

			if (canStartSequence(cmd) == true)
			{
				// start sequence
				//
				commandsInSequence = 1;

				firstSequenceCmd = it;
				lastSequenceCmd = it;
			}
			else
			{
				optiCode.append(cmd);
			}
		}

		m_srcCode.swap(optiCode);

		return true;
	}

	const ModuleLogicCompiler& SequenceOptimization::compiler() const
	{
		return m_compiler;
	}

	bool SequenceOptimization::hasRequiredCommands(const std::vector<LmCommandCode>& requiredCmdCodes) const
	{
		for(const LmCommandCode cmd : requiredCmdCodes)
		{
			if (m_compiler.getLmDescription()->commandPtr(cmd) == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	// ---------------------------------------------------------------------------------------
	//
	// SequentialMovesOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	SequentialMovesOptimization::SequentialMovesOptimization(ModuleLogicCompiler& compiler,
															 CodeSnippet& srcCode,
															 const LmMemoryMap& memoryMap) :
		SequenceOptimization(CodeOptimizationType::SequentialMoves, compiler, srcCode),
		m_memoryMap(memoryMap)
	{
	}

	bool SequentialMovesOptimization::isOptimizationPossible() const
	{
		return true;
	}

	void SequentialMovesOptimization::reinitVars()
	{
		m_sequenceMoveSizeW = 0;
		m_sequenceStartSrcAddr = 0;
		m_sequenceStartDestAddr = 0;
	}

	bool SequentialMovesOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (isAppropriateMoveCmd(cmd) == false)
		{
			return false;
		}

		// init sequence vars
		//
		m_sequenceMoveSizeW = cmd.getMoveSizeW();
		m_sequenceStartSrcAddr = cmd.srcAddr();
		m_sequenceStartDestAddr = cmd.destAddr();

		return true;
	}

	bool SequentialMovesOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		Q_ASSERT(cmd.isCommand() == true);
		Q_ASSERT(inSequence() == true);

		if (isAppropriateMoveCmd(cmd) == false)
		{
			return false;
		}

		quint16 srcAddr = cmd.srcAddr();
		quint16 destAddr = cmd.destAddr();

		if (m_memoryMap.addressInBitMemory(destAddr) == true)
		{
			return false;
		}

		if (m_sequenceStartSrcAddr + m_sequenceMoveSizeW == srcAddr &&
			m_sequenceStartDestAddr + m_sequenceMoveSizeW == destAddr)
		{
			m_sequenceMoveSizeW += cmd.getMoveSizeW();
			return true;
		}

		return false;
	}

	bool SequentialMovesOptimization::canOptimize() const
	{
		return inSequence();
	}

	bool SequentialMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_sequenceMoveSizeW <= 1)
		{
			Q_ASSERT(false);
			return false;
		}

		code.clear();

		if (m_sequenceMoveSizeW == 2)
		{
			code << CodeItem().mov32(m_sequenceStartDestAddr,
									 m_sequenceStartSrcAddr);
		}
		else
		{
			code << CodeItem().movMem(m_sequenceStartDestAddr,
									 m_sequenceStartSrcAddr,
									 m_sequenceMoveSizeW);
		}

		return true;
	}

	bool SequentialMovesOptimization::isAppropriateMoveCmd(const CodeItem& cmd) const
	{
		return 	cmd.isMoveCmd() ||
				cmd.isMove32Cmd() ||
				cmd.isMoveMemCmd();
	}

	bool SequentialMovesOptimization::inSequence() const
	{
		return (m_sequenceMoveSizeW != 0);
	}

	// ---------------------------------------------------------------------------------------
	//
	// SequentialConstMovesOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	SequentialConstMovesOptimization::SequentialConstMovesOptimization(ModuleLogicCompiler& compiler,
															 CodeSnippet& srcCode,
															 const LmMemoryMap& memoryMap) :
		SequenceOptimization(CodeOptimizationType::SequentialConstMoves, compiler, srcCode),
		m_memoryMap(memoryMap)
	{
	}

	bool SequentialConstMovesOptimization::isOptimizationPossible() const
	{
		return true;
	}

	void SequentialConstMovesOptimization::reinitVars()
	{
		m_moveConst = 0;
		m_sequenceMoveSizeW = 0;
		m_sequenceStartDestAddr = 0;
	}

	bool SequentialConstMovesOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (isAppropriateMoveCmd(cmd) == false)
		{
			return false;
		}

		if (cmd.isMoveConst32Cmd() == true)
		{
			quint32 const32 = cmd.getConst32();

			if (( const32 >> 16) != (const32 & 0xFFFF))		// upper and lower words of const32 should be equal
			{
				return false;
			}

			m_moveConst = const32 & 0xFFFF;
		}
		else
		{
			m_moveConst = cmd.getConst16();
		}

		// init sequence vars
		//
		m_sequenceMoveSizeW = cmd.getMoveSizeW();
		m_sequenceStartDestAddr = cmd.destAddr();

		return true;
	}

	bool SequentialConstMovesOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		Q_ASSERT(cmd.isCommand() == true);
		Q_ASSERT(inSequence() == true);

		if (isAppropriateMoveCmd(cmd) == false)
		{
			return false;
		}

		if (cmd.isMoveConst32Cmd() == true)
		{
			quint32 const32 = cmd.getConst32();

			if (m_moveConst != (const32 >> 16) ||
				m_moveConst != (const32 & 0xFFFF))
			{
				return false;
			}
		}
		else
		{
			if (m_moveConst != cmd.getConst16())
			{
				return false;
			}
		}

		quint16 destAddr = cmd.destAddr();

		if (m_memoryMap.addressInBitMemory(destAddr) == true)
		{
			return false;
		}

		if (m_sequenceStartDestAddr + m_sequenceMoveSizeW == destAddr)
		{
			m_sequenceMoveSizeW += cmd.getMoveSizeW();
			return true;
		}

		return false;
	}

	bool SequentialConstMovesOptimization::canOptimize() const
	{
		return inSequence();
	}

	bool SequentialConstMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_sequenceMoveSizeW <= 1)
		{
			Q_ASSERT(false);
			return false;
		}

		code.clear();

		if (m_sequenceMoveSizeW == 2)
		{
			code << CodeItem().movConstUInt32(m_sequenceStartDestAddr,
									  (static_cast<quint32>(m_moveConst) << 16) | m_moveConst);
		}
		else
		{
			code << CodeItem().setMem(m_sequenceStartDestAddr,
									  static_cast<int>(m_moveConst),
									  m_sequenceMoveSizeW);
		}

		return true;
	}

	bool SequentialConstMovesOptimization::isAppropriateMoveCmd(const CodeItem& cmd) const
	{
		return 	cmd.isMoveConstCmd() ||
				cmd.isMoveConst32Cmd() ||
				cmd.isSetMemCmd();
	}

	bool SequentialConstMovesOptimization::inSequence() const
	{
		return (m_sequenceMoveSizeW != 0);
	}

	// ---------------------------------------------------------------------------------------
	//
	// SequentialBitMovesOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	SequentialBitMovesOptimization::SequentialBitMovesOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::SequentialBitMoves, compiler, srcCode),
		m_bitAccAddr(compiler.bitAccumulatorAddress())
	{
	}

	bool SequentialBitMovesOptimization::isOptimizationPossible() const
	{
		return true;
	}

	void SequentialBitMovesOptimization::reinitVars()
	{
		m_srcAddr = 0;
		m_destAddr = 0;

		m_bitField = 0;
		m_bitCount = 0;

		m_directMoveDestAddr = BAD_ADDRESS;
	}

	bool SequentialBitMovesOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (cmd.isMoveBitCmd() == false)
		{
			return false;
		}

		auto srcAddr = cmd.srcBitAddr();
		auto destAddr = cmd.destBitAddr();

		if (srcAddr.bit() != destAddr.bit())
		{
			return false;
		}

		m_srcAddr = srcAddr.offset();
		m_destAddr = destAddr.offset();

		m_bitField = 0;
		m_bitCount = 0;

		return setBit(srcAddr.bit());
	}

	bool SequentialBitMovesOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		if (cmd.isMoveBitCmd() == true)
		{
			auto srcAddr = cmd.srcBitAddr();
			auto destAddr = cmd.destBitAddr();

			if (srcAddr.bit() != destAddr.bit())
			{
				return false;
			}

			if (m_srcAddr != srcAddr.offset() ||
				m_destAddr != destAddr.offset())
			{
				return false;
			}

			return setBit(srcAddr.bit());
		}

		// include command: mov memAddr, bitAcc
		//
		if (cmd.isMoveCmd() && cmd.srcAddr() == m_bitAccAddr &&		// this is a move command from bitAcc to memory
			m_destAddr == m_bitAccAddr &&							// bits sequence was written in bit accumulator
			m_bitCount == 16 &&	m_bitField == 0xFFFF)				// all bits already written
		{
			m_directMoveDestAddr = cmd.destAddr();
			return true;
		}

		return false;
	}

	bool SequentialBitMovesOptimization::canOptimize() const
	{
		return m_bitCount == 16 && m_bitField == 0xFFFF;
	}

	bool SequentialBitMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_directMoveDestAddr != BAD_ADDRESS)
		{
			code << CodeItem().mov(m_directMoveDestAddr, m_srcAddr);
		}
		else
		{
			code << CodeItem().mov(m_destAddr, m_srcAddr);
		}

		return true;
	}

	bool SequentialBitMovesOptimization::setBit(int bitNo)
	{
		if (bitNo < 0 || bitNo > 15)
		{
			Q_ASSERT(false);
			return false;
		}

		quint16 mask = 0x0001 << bitNo;

		if ((m_bitField & mask) != 0)
		{
			Q_ASSERT(false);
			return false;
		}

		m_bitField |= mask;
		m_bitCount++;

		return true;
	}

	bool SequentialBitMovesOptimization::inSequence() const
	{
		return m_bitField != 0;
	}

	// ---------------------------------------------------------------------------------------
	//
	// BitFillingOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	BitFillingOptimization::BitFillingOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::BitFilling, compiler, srcCode),
		m_bitAccAddr(compiler.bitAccumulatorAddress())
	{
	}

	bool BitFillingOptimization::isOptimizationPossible() const
	{
		return true;
	}

	void BitFillingOptimization::reinitVars()
	{
		m_srcBitAddr.reset();
		m_destAddr = 0;

		m_bitField = 0;
		m_bitCount = 0;

		m_directMoveDestAddr = BAD_ADDRESS;
	}

	bool BitFillingOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (cmd.isMoveBitCmd() == false)
		{
			return false;
		}

		m_srcBitAddr = cmd.srcBitAddr();
		m_destAddr = cmd.destBitAddr().offset();

		m_bitField = 0;
		m_bitCount = 0;

		return setBit(cmd.destBitAddr().bit());
	}

	bool BitFillingOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		if (cmd.isMoveBitCmd() == true)
		{
			auto srcBitAddr = cmd.srcBitAddr();
			auto destBitAddr = cmd.destBitAddr();

			if (srcBitAddr != m_srcBitAddr ||
				destBitAddr.offset() != m_destAddr)
			{
				return false;
			}

			return setBit(destBitAddr.bit());
		}

		// include command: mov memAddr, bitAcc
		//
		if (cmd.isMoveCmd() && cmd.srcAddr() == m_bitAccAddr &&		// this is a move command from bitAcc to memory
			m_destAddr == m_bitAccAddr &&							// bits sequence was written in bit accumulator
			m_bitCount == 16 &&	m_bitField == 0xFFFF)				// all bits already written
		{
			m_directMoveDestAddr = cmd.destAddr();
			return true;
		}

		return false;
	}

	bool BitFillingOptimization::canOptimize() const
	{
		return m_bitCount == 16 && m_bitField == 0xFFFF;
	}

	bool BitFillingOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_directMoveDestAddr != BAD_ADDRESS)
		{
			code << CodeItem().fillb(Address16(m_directMoveDestAddr, 0), m_srcBitAddr);
		}
		else
		{
			code << CodeItem().fillb(Address16(m_destAddr, 0), m_srcBitAddr);
		}

		return true;
	}

	bool BitFillingOptimization::setBit(int bitNo)
	{
		if (bitNo < 0 || bitNo > 15)
		{
			Q_ASSERT(false);
			return false;
		}

		quint16 mask = 0x0001 << bitNo;

		if ((m_bitField & mask) != 0)
		{
			Q_ASSERT(false);
			return false;
		}

		m_bitField |= mask;
		m_bitCount++;

		return true;
	}

	bool BitFillingOptimization::inSequence() const
	{
		return m_bitField != 0;
	}

	// ---------------------------------------------------------------------------------------
	//
	// BitAccNotOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	BitAccNotOptimization::BitAccNotOptimization(ModuleLogicCompiler& compiler,
						  CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::BitAccNot, compiler, srcCode)
	{

	}

	bool BitAccNotOptimization::isOptimizationPossible() const
	{
		static const std::vector<LmCommandCode> requiredCommands =
		{
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::NOT,
			LmCommand::MOVB_ADDR_ACC
		};

		return hasRequiredCommands(requiredCommands);
	}

	void BitAccNotOptimization::reinitVars()
	{
		m_srcBitAddr.reset();
		m_destBitAddr.reset();
		m_sequenceState = -1;
	}

	bool BitAccNotOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (!(cmd.isWriteFuncBlockBitCmd() &&
			cmd.getFbType() == NOT_AFB_OPCODE &&
			cmd.getFbInstance() == 0 &&
			cmd.getFbParamNo() == AFB_NOT_IN_PIN_INDEX))
		{
			return false;
		}

		m_sequenceState = 0;
		m_srcBitAddr = cmd.srcBitAddr();
		return true;
	}

	bool BitAccNotOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		switch(m_sequenceState)
		{
		case 0:
			if (cmd.isStartAfbCmd() &&
				cmd.getFbType() == NOT_AFB_OPCODE &&
				cmd.getFbInstance() == 0)
			{
				m_sequenceState = 1;
				return true;
			}

			break;

		case 1:
			if (cmd.isReadFuncBlockBitCmd() &&
				cmd.getFbType() == NOT_AFB_OPCODE &&
				cmd.getFbInstance() == 0 &&
				cmd.getFbParamNo() == AFB_NOT_OUT_PIN_INDEX)
			{
				m_sequenceState = 2;
				m_destBitAddr = cmd.destBitAddr();
				return true;
			}

			break;

		default: ;
		}

		return false;
	}

	bool BitAccNotOptimization::canOptimize() const
	{
		return m_sequenceState == 2;
	}

	bool BitAccNotOptimization::getReplacementCode(CodeSnippet& code)
	{
		code << CodeItem().movBitAccAddr(m_srcBitAddr);
		code << CodeItem().notAcc();
		code << CodeItem().movBitAddrAcc(m_destBitAddr);

		return true;
	}

	bool BitAccNotOptimization::inSequence() const
	{
		return m_sequenceState >= 0 && m_sequenceState <= 2;
	}

	// ---------------------------------------------------------------------------------------
	//
	// SequentialAccBitMovesOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	SequentialAccBitMovesOptimization::SequentialAccBitMovesOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::SequentialAccBitMoves, compiler, srcCode)
	{
	}

	bool SequentialAccBitMovesOptimization::isOptimizationPossible() const
	{
		m_constBit0Addr = compiler().constBit0Addr();
		m_constBit1Addr = compiler().constBit1Addr();

		static const std::vector<LmCommandCode> requiredCommands =
		{
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::MOV_ADDR_ACC,
			LmCommand::RESET,
			LmCommand::LSHIFT0,
			LmCommand::LSHIFT1,
		};

		return hasRequiredCommands(requiredCommands);
	}

	void SequentialAccBitMovesOptimization::reinitVars()
	{
		m_sequenceState = -1;
		m_prevDestAccAddr = BAD_ADDRESS;
		m_destAccAddr = BAD_ADDRESS;

		for(Address16& bitSrcAddr : m_bitSrcAddrs)
		{
			bitSrcAddr.reset();
		}

		m_movedBitCount = 0;
		m_directMoveDestAddr = BAD_ADDRESS;
	}

	bool SequentialAccBitMovesOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (cmd.isMoveBitConstCmd() && cmd.destBitAddr() == Address16(54280, 0))
		{
			DEBUG_STOP;
		}

		if (cmd.isMoveConstCmd() == true &&
			cmd.getConst16() == 0 &&
			m_sequenceState == -1)
		{
			m_prevDestAccAddr = cmd.destAddr();
			m_sequenceState = 0;
			return true;
		}

		if ((cmd.isMoveBitCmd() == false &&
			cmd.isMoveBitConstCmd() == false) ||
			m_sequenceState != -1 ||
			m_destAccAddr != BAD_ADDRESS ||
			m_movedBitCount != 0)
		{
			return false;
		}

		Address16 destBitAddr = cmd.destBitAddr();

		if (destBitAddr.bit() != 0)
		{
			return false;
		}

		m_destAccAddr = destBitAddr.offset();

		m_sequenceState = 1;

		if (cmd.isMoveBitCmd() == true)
		{
			m_bitSrcAddrs[m_movedBitCount] = cmd.srcBitAddr();
		}
		else
		{
			if (cmd.getConstBit() == 0)
			{
				m_bitSrcAddrs[m_movedBitCount] = m_constBit0Addr;
			}
			else
			{
				m_bitSrcAddrs[m_movedBitCount] = m_constBit1Addr;
			}
		}

		m_movedBitCount++;

		return true;
	}

	bool SequentialAccBitMovesOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		if ((cmd.isMoveBitCmd() == true || cmd.isMoveBitConstCmd() == true) &&
			(m_sequenceState == 0 || m_sequenceState == 1))
		{
			if (m_sequenceState == 0)
			{
				Address16 destBitAddr = cmd.destBitAddr();

				if (m_prevDestAccAddr != destBitAddr.offset() || destBitAddr.bit() != 0)
				{
					return false;
				}

				m_destAccAddr = destBitAddr.offset();
				m_sequenceState = 1;
			}
			else
			{
				Address16 destBitAddr = cmd.destBitAddr();

				if (m_destAccAddr != destBitAddr.offset() ||
					m_movedBitCount != destBitAddr.bit())
				{
					if (m_movedBitCount == 16)
					{
						m_sequenceState = 2;						// sequence finished
					}

					return true;
				}
			}

			if (cmd.isMoveBitCmd() == true)
			{
				m_bitSrcAddrs[m_movedBitCount] = cmd.srcBitAddr();
			}
			else
			{
				if (cmd.getConstBit() == 0)
				{
					m_bitSrcAddrs[m_movedBitCount] = m_constBit0Addr;
				}
				else
				{
					m_bitSrcAddrs[m_movedBitCount] = m_constBit1Addr;
				}
			}

			m_movedBitCount++;
			return true;
		}

		if (cmd.isMoveCmd() == true &&
			m_sequenceState == 1 &&
			m_destAccAddr == cmd.srcAddr())
		{
			m_directMoveDestAddr = cmd.destAddr();
			m_sequenceState = 2;						// sequence finished
			return true;
		}

		// any another command
		//
		if (m_sequenceState == 1 &&
			m_movedBitCount == 16)
		{
			m_sequenceState = 2;						// sequence finished
		}

		return false;
	}

	bool SequentialAccBitMovesOptimization::canOptimize() const
	{
		return m_sequenceState == 2 && m_movedBitCount > 0;
	}

	bool SequentialAccBitMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_movedBitCount < 16)
		{
			code << CodeItem().resetAcc();
		}

		for(int i = m_movedBitCount - 1; i >= 0; i--)
		{
			if (m_bitSrcAddrs[i] == m_constBit0Addr)
			{
				code << CodeItem().lshift0Acc();
				continue;
			}

			if (m_bitSrcAddrs[i] == m_constBit1Addr)
			{
				code << CodeItem().lshift1Acc();
				continue;
			}

			if (m_bitSrcAddrs[i].isValid() == false)
			{
				Q_ASSERT(false);
				return false;
			}

			code << CodeItem().movBitAccAddr(m_bitSrcAddrs[i]);
		}

		if (m_directMoveDestAddr != BAD_ADDRESS)
		{
			code << CodeItem().movAddrAcc(m_directMoveDestAddr);
		}
		else
		{
			code << CodeItem().movAddrAcc(m_destAccAddr);
		}

		return true;
	}

	bool SequentialAccBitMovesOptimization::inSequence() const
	{
		return m_sequenceState >= 0 && m_sequenceState <= 2;
	}

	// ---------------------------------------------------------------------------------------
	//
	// BitAccAndOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	BitAccAndOptimization::BitAccAndOptimization(ModuleLogicCompiler& compiler,
						  CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::BitAccAnd, compiler, srcCode)
	{

	}

	bool BitAccAndOptimization::isOptimizationPossible() const
	{
		static const std::vector<LmCommandCode> requiredCommands =
		{
			LmCommand::SET,
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::AND,
			LmCommand::MOVB_ADDR_ACC
		};

		if (hasRequiredCommands(requiredCommands) == false)
		{
			return false;
		}

		std::set<QString> inputsCaptions;

		for(int i = 1; i <= 16; i++)
		{
			inputsCaptions.insert(QString("in_%1").arg(i));
		}

		const UalAfbsMap& ualAfbs = compiler().ualAfbs();

		for(const UalAfb* ualAfb : ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			if (ualAfb->caption() != QStringLiteral("and"))
			{
				continue;
			}

			bool ok = false;

			int iConfValue = ualAfb->getParamIntValueByOpName("i_conf", &ok);

			if (ok == false ||
				ualAfb->opcode() != LOGIC_AFB_OPCODE ||
				iConfValue != LOGIC_CONF_AND)
			{
				Q_ASSERT(false);
				continue;
			}

			int operandCount = ualAfb->getParamIntValueByOpName("i_oprd_quant", &ok);

			m_afbInstances.insert({ualAfb->instance(), operandCount});

			const std::vector<LogicPin>& inputs = ualAfb->inputs();

			for(const LogicPin& input : inputs)
			{
				if (inputsCaptions.contains(input.caption()) == true)
				{
					m_inputIndexes.insert(input.afbOperandIndex());
				}
			}

			if (m_outputIndex == -1)
			{
				const std::vector<LogicPin>& outputs = ualAfb->outputs();

				if (outputs.size() == 1)
				{
					m_outputIndex = outputs[0].afbOperandIndex();
				}
			}
		}

		m_constBit0Addr = compiler().constBit0Addr();
		m_constBit1Addr = compiler().constBit1Addr();

		return true;
	}

	void BitAccAndOptimization::reinitVars()
	{
		m_sequenceState = -1;
		m_loadBitCount = 0;
		m_afbInstance = -1;
		m_srcBitAddrs.clear();
		m_destBitAddr.reset();
	}

	bool BitAccAndOptimization::canStartSequence(const CodeItem& cmd)
	{
		if ((cmd.isWriteFuncBlockBitCmd() ||
			 cmd.isWriteFuncBlockConstCmd()) &&
			 cmd.getFbType() == LOGIC_AFB_OPCODE &&
			  m_afbInstances.contains(cmd.getFbInstance()) == true &&
			  m_inputIndexes.contains(cmd.getFbParamNo()) == true)
		{
			m_afbInstance = cmd.getFbInstance();

			return processCommand(cmd);
		}

		return false;
	}

	bool BitAccAndOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		switch(m_sequenceState)
		{
		case 0:		// loading bits state

			if (cmd.isStartAfbCmd() &&
				cmd.getFbType() == LOGIC_AFB_OPCODE  &&
				cmd.getFbInstance() == m_afbInstance)
			{
				m_sequenceState = 1;
				return true;
			}

			if ((cmd.isWriteFuncBlockBitCmd() ||
				 cmd.isWriteFuncBlockConstCmd()) &&
				 cmd.getFbType() == LOGIC_AFB_OPCODE &&
				 cmd.getFbInstance() == m_afbInstance &&
				 m_inputIndexes.contains(cmd.getFbParamNo()) == true)
			{
				return processCommand(cmd);
			}

			break;

		case 1:	// afb started state

			if (cmd.isReadFuncBlockBitCmd() &&
				cmd.getFbType() == LOGIC_AFB_OPCODE &&
				cmd.getFbInstance() == m_afbInstance &&
				cmd.getFbParamNo() == m_outputIndex)
			{
				m_destBitAddr = cmd.destBitAddr();
				m_sequenceState = 2;
				return true;
			}

			break;

		default: ;
		}

		return false;
	}

	bool BitAccAndOptimization::canOptimize() const
	{
		if (m_sequenceState != 2)
		{
			return false;
		}

		auto it = m_afbInstances.find(m_afbInstance);

		if (it == m_afbInstances.end())
		{
			Q_ASSERT(false);
			return false;
		}

		if (it->second != m_loadBitCount)
		{
			Q_ASSERT(false);
			return false;
		}

		return true;
	}

	bool BitAccAndOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_srcBitAddrs.contains(m_constBit0Addr) == true)
		{
			code << CodeItem().movBitConst(m_destBitAddr, 0);
			return true;
		}

		code << CodeItem().setAcc();

		for(const Address16& srcAddr : m_srcBitAddrs)
		{
			code << CodeItem().movBitAccAddr(srcAddr);
		}

		code << CodeItem().andAcc();
		code << CodeItem().movBitAddrAcc(m_destBitAddr);

		return true;
	}

	bool BitAccAndOptimization::inSequence() const
	{
		return m_sequenceState >= 0 && m_sequenceState <= 2;
	}

	bool BitAccAndOptimization::processCommand(const CodeItem& cmd)
	{
		if (cmd.isWriteFuncBlockBitCmd() == true)
		{
			m_srcBitAddrs.insert(cmd.srcBitAddr());
		}
		else
		{
			if (cmd.isWriteFuncBlockConstCmd() == true)
			{
				quint16 constValue = cmd.getConst16();

				switch(constValue)
				{
				case 0:
					m_srcBitAddrs.insert(m_constBit0Addr);
					break;

				case 1:
					m_srcBitAddrs.insert(m_constBit1Addr);
					break;

				default:
					Q_ASSERT(false);
					return false;
				}
			}
			else
			{
				Q_ASSERT(false);
				return false;
			}
		}

		m_loadBitCount++;
		Q_ASSERT(m_loadBitCount <= 16);
		m_sequenceState = 0;
		return true;
	}

	// ---------------------------------------------------------------------------------------
	//
	// BitAccOrOptimization class implementation
	//
	// ---------------------------------------------------------------------------------------

	BitAccOrOptimization::BitAccOrOptimization(ModuleLogicCompiler& compiler,
						  CodeSnippet& srcCode) :
		SequenceOptimization(CodeOptimizationType::BitAccOr, compiler, srcCode)
	{

	}

	bool BitAccOrOptimization::isOptimizationPossible() const
	{
		static const std::vector<LmCommandCode> requiredCommands =
		{
			LmCommand::RESET,
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::OR,
			LmCommand::MOVB_ADDR_ACC
		};

		if (hasRequiredCommands(requiredCommands) == false)
		{
			return false;
		}

		std::set<QString> inputsCaptions;

		for(int i = 1; i <= 16; i++)
		{
			inputsCaptions.insert(QString("in_%1").arg(i));
		}

		const UalAfbsMap& ualAfbs = compiler().ualAfbs();

		for(const UalAfb* ualAfb : ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			if (ualAfb->caption() != QStringLiteral("or"))
			{
				continue;
			}

			bool ok = false;

			int iConfValue = ualAfb->getParamIntValueByOpName("i_conf", &ok);

			if (ok == false ||
				ualAfb->opcode() != LOGIC_AFB_OPCODE ||
				iConfValue != LOGIC_CONF_OR)
			{
				Q_ASSERT(false);
				continue;
			}

			int operandCount = ualAfb->getParamIntValueByOpName("i_oprd_quant", &ok);

			m_afbInstances.insert({ualAfb->instance(), operandCount});

			const std::vector<LogicPin>& inputs = ualAfb->inputs();

			for(const LogicPin& input : inputs)
			{
				if (inputsCaptions.contains(input.caption()) == true)
				{
					m_inputIndexes.insert(input.afbOperandIndex());
				}
			}

			if (m_outputIndex == -1)
			{
				const std::vector<LogicPin>& outputs = ualAfb->outputs();

				if (outputs.size() == 1)
				{
					m_outputIndex = outputs[0].afbOperandIndex();
				}
			}
		}

		m_constBit0Addr = compiler().constBit0Addr();
		m_constBit1Addr = compiler().constBit1Addr();

		return true;
	}

	void BitAccOrOptimization::reinitVars()
	{
		m_sequenceState = -1;
		m_loadBitCount = 0;
		m_afbInstance = -1;
		m_srcBitAddrs.clear();
		m_destBitAddr.reset();
	}

	bool BitAccOrOptimization::canStartSequence(const CodeItem& cmd)
	{
		if ((cmd.isWriteFuncBlockBitCmd() ||
			 cmd.isWriteFuncBlockConstCmd()) &&
			 cmd.getFbType() == LOGIC_AFB_OPCODE &&
			  m_afbInstances.contains(cmd.getFbInstance()) == true &&
			  m_inputIndexes.contains(cmd.getFbParamNo()) == true)
		{
			m_afbInstance = cmd.getFbInstance();

			return processCommand(cmd);
		}

		return false;
	}

	bool BitAccOrOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		switch(m_sequenceState)
		{
		case 0:		// loading bits state

			if (cmd.isStartAfbCmd() &&
				cmd.getFbType() == LOGIC_AFB_OPCODE  &&
				cmd.getFbInstance() == m_afbInstance)
			{
				m_sequenceState = 1;
				return true;
			}

			if ((cmd.isWriteFuncBlockBitCmd() ||
				 cmd.isWriteFuncBlockConstCmd()) &&
				 cmd.getFbType() == LOGIC_AFB_OPCODE &&
				 cmd.getFbInstance() == m_afbInstance &&
				 m_inputIndexes.contains(cmd.getFbParamNo()) == true)
			{
				return processCommand(cmd);
			}

			break;

		case 1:	// afb started state

			if (cmd.isReadFuncBlockBitCmd() &&
				cmd.getFbType() == LOGIC_AFB_OPCODE &&
				cmd.getFbInstance() == m_afbInstance &&
				cmd.getFbParamNo() == m_outputIndex)
			{
				m_destBitAddr = cmd.destBitAddr();
				m_sequenceState = 2;
				return true;
			}

			break;

		default: ;
		}

		return false;
	}

	bool BitAccOrOptimization::canOptimize() const
	{
		if (m_sequenceState != 2)
		{
			return false;
		}

		auto it = m_afbInstances.find(m_afbInstance);

		if (it == m_afbInstances.end())
		{
			Q_ASSERT(false);
			return false;
		}

		if (it->second != m_loadBitCount)
		{
			Q_ASSERT(false);
			return false;
		}

		return true;
	}

	bool BitAccOrOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_srcBitAddrs.contains(m_constBit1Addr) == true)
		{
			code << CodeItem().movBitConst(m_destBitAddr, 1);
			return true;
		}

		code << CodeItem().resetAcc();

		for(const Address16& srcAddr : m_srcBitAddrs)
		{
			code << CodeItem().movBitAccAddr(srcAddr);
		}

		code << CodeItem().orAcc();
		code << CodeItem().movBitAddrAcc(m_destBitAddr);

		return true;
	}

	bool BitAccOrOptimization::inSequence() const
	{
		return m_sequenceState >= 0 && m_sequenceState <= 2;
	}

	bool BitAccOrOptimization::processCommand(const CodeItem& cmd)
	{
		if (cmd.isWriteFuncBlockBitCmd() == true)
		{
			m_srcBitAddrs.insert(cmd.srcBitAddr());
		}
		else
		{
			if (cmd.isWriteFuncBlockConstCmd() == true)
			{
				quint16 constValue = cmd.getConst16();

				switch(constValue)
				{
				case 0:
					m_srcBitAddrs.insert(m_constBit0Addr);
					break;

				case 1:
					m_srcBitAddrs.insert(m_constBit1Addr);
					break;

				default:
					Q_ASSERT(false);
					return false;
				}
			}
			else
			{
				Q_ASSERT(false);
				return false;
			}
		}

		m_loadBitCount++;
		Q_ASSERT(m_loadBitCount <= 16);
		m_sequenceState = 0;
		return true;
	}
}
