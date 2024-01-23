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
											getReplacementCode(replacementCode) == true &&
											replacementCode.codeSizeW(m_compiler.getLmDescription()) <
														m_srcCode.codeSizeW(m_compiler.getLmDescription(),
																			firstSequenceCmd,
																			lastSequenceCmd))
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
}
