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
			return "None";

		case CodeOptimizationType::SequentialMoves:
			return "SequentialMoves";

		case CodeOptimizationType::SequentialConstMoves:
			return "SequentialConstMoves";

		case CodeOptimizationType::SequentialBitMoves:
			return "SequentialBitMoves";

		case CodeOptimizationType::BitFilling:
			return "BitFilling";

		case CodeOptimizationType::BitAccNot:
			return "BitAccNot";

		case CodeOptimizationType::SequentialAccBitMoves:
			return "SequentialAccBitMoves";

		default:
			Q_ASSERT(false);
		}

		return QString();
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
										if (canOptimize() == true)
										{
											CodeSnippet replacementCode;

											getReplacementCode(replacementCode);

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

	void SequentialMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		Q_ASSERT(m_sequenceMoveSizeW > 1);

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

	void SequentialConstMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		Q_ASSERT(m_sequenceMoveSizeW > 1);

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

	void SequentialBitMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_directMoveDestAddr != BAD_ADDRESS)
		{
			code << CodeItem().mov(m_directMoveDestAddr, m_srcAddr);
		}
		else
		{
			code << CodeItem().mov(m_destAddr, m_srcAddr);
		}
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

	void BitFillingOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_directMoveDestAddr != BAD_ADDRESS)
		{
			code << CodeItem().fillb(Address16(m_directMoveDestAddr, 0), m_srcBitAddr);
		}
		else
		{
			code << CodeItem().fillb(Address16(m_destAddr, 0), m_srcBitAddr);
		}
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
		const std::shared_ptr<Afb::AfbElement> notElem = compiler().getLmDescription()->afbElement("not");

		if (notElem == nullptr ||
			notElem->opCode() > ModuleLogicCompiler::MAX_AFB_OPCODE)
		{
			return false;
		}

		m_afbNotOpcode = notElem->opCode();

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
		m_sequenceIndex = -1;
	}

	bool BitAccNotOptimization::canStartSequence(const CodeItem& cmd)
	{
		if (!(cmd.isWriteFuncBlockBitCmd() &&
			cmd.getFbType() == m_afbNotOpcode &&
			cmd.getFbInstance() == 0 &&
			cmd.getFbParamNo() == AFB_NOT_IN_PIN_INDEX))
		{
			return false;
		}

		m_sequenceIndex = 0;
		m_srcBitAddr = cmd.srcBitAddr();
		return true;
	}

	bool BitAccNotOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		switch(m_sequenceIndex)
		{
		case 0:
			if (cmd.isStartAfbCmd() &&
				cmd.getFbType() == m_afbNotOpcode &&
				cmd.getFbInstance() == 0)
			{
				m_sequenceIndex++;
				return true;
			}

			break;

		case 1:
			if (cmd.isReadFuncBlockBitCmd() &&
				cmd.getFbType() == m_afbNotOpcode &&
				cmd.getFbInstance() == 0 &&
				cmd.getFbParamNo() == AFB_NOT_OUT_PIN_INDEX)
			{
				m_sequenceIndex++;
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
		return m_sequenceIndex == 2;
	}

	void BitAccNotOptimization::getReplacementCode(CodeSnippet& code)
	{
		code << CodeItem().movBitAccAddr(m_srcBitAddr);
		code << CodeItem().notAcc();
		code << CodeItem().movBitAddrAcc(m_destBitAddr);
	}

	bool BitAccNotOptimization::inSequence() const
	{
		return m_sequenceIndex >= 0 && m_sequenceIndex <= 2;
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
		static const std::vector<LmCommandCode> requiredCommands =
		{
			LmCommand::RESET,
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::MOV_ADDR_ACC
		};

		return hasRequiredCommands(requiredCommands);
	}

	void SequentialAccBitMovesOptimization::reinitVars()
	{
		m_sequenceState = -1;
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
		if (cmd.isMoveConstCmd() == true &&
			cmd.getConst16() == 0 &&
			m_sequenceState == -1)
		{
			m_sequenceState = 0;
			return true;
		}

		if (cmd.isMoveBitCmd() == false ||
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

		m_bitSrcAddrs[m_movedBitCount] = cmd.srcBitAddr();

		m_movedBitCount++;

		return true;
	}

	bool SequentialAccBitMovesOptimization::isSequenceContinue(const CodeItem& cmd)
	{
		if (cmd.isMoveBitCmd() == true &&
			(m_sequenceState == 0 || m_sequenceState == 1))
		{
			if (m_sequenceState == 0)
			{
				Address16 destBitAddr = cmd.destBitAddr();

				if (destBitAddr.bit() != 0)
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
					m_movedBitCount != destBitAddr.bit() ||
					m_movedBitCount >= 16)
				{
					return false;
				}
			}

			m_bitSrcAddrs[m_movedBitCount] = cmd.srcBitAddr();
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

		return false;
	}

	bool SequentialAccBitMovesOptimization::canOptimize() const
	{
		return m_sequenceState == 2 && m_movedBitCount > 0;
	}

	void SequentialAccBitMovesOptimization::getReplacementCode(CodeSnippet& code)
	{
		if (m_movedBitCount < 16)
		{
			code << CodeItem().resetAcc();
		}

		for(int i = m_movedBitCount - 1; i >= 0; i--)
		{
			code << CodeItem().movBitAccAddr(m_bitSrcAddrs[i]);
		}

		code << CodeItem().movAddrAcc(m_directMoveDestAddr);
	}

	bool SequentialAccBitMovesOptimization::inSequence() const
	{
		return m_sequenceState >= 0 && m_sequenceState <= 2;
	}

}
