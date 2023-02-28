#include "CodeOptimization.h"
#include "ModuleLogicCompiler.h"

namespace Builder
{
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
		int commandsInSequence = 0;

		CodeSnippetConstIterator firstSequenceCmd;
		CodeSnippetConstIterator lastSequenceCmd;

		CodeSnippet optiCode;

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

										getReplacementCode(replacementCode);

										m_compiler.optimizeCode(m_optimizationType,
																m_srcCode,
																firstSequenceCmd,
																lastSequenceCmd,
																optiCode,
																replacementCode);
									}
									else
									{
										// sequenceSize == 1
										//
										optiCode.append(*firstSequenceCmd);
									}

									sequenceReinitVars();
								}
							};

		auto inSequence =	[&]() -> bool
							{
								return firstSequenceCmd != m_srcCode.end();
							};

		sequenceReinitVars();

		for(CodeSnippetConstIterator it = m_srcCode.begin(); it != m_srcCode.end(); it++)
		{
			const CodeItem& cmd = *it;

			if (cmd.isComment() == true)
			{
				if (inSequence() == true)
				{
					// continue sequence
					//
					lastSequenceCmd = it;
				}
				else
				{
					// not in sequence - copy comment to optimized code
					//
					optiCode.append(cmd);
				}

				continue;
			}

			if (inSequence() == true)
			{
				if (isSequenceContinue(cmd) == true)
				{
					// continue sequence
					//
					commandsInSequence++;
					lastSequenceCmd = it;
				}
				else
				{
					finalizeSequence();
					optiCode.append(cmd);
				}

				continue;
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

}
