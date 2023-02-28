#pragma once

#include "ApplicationLogicCode.h"

namespace Builder
{
	enum class CodeOptimizationType
	{
		None,
		SequentialMoves,
	};

	struct OptimizationInfo
	{
		OptimizationInfo() = delete;

		OptimizationInfo(CodeOptimizationType t) : type(t) { }

		CodeOptimizationType type = CodeOptimizationType::None;
		int optimizationsCount = 0;
		int decreasingCodeSizeW = 0;
	};

	class ModuleLogicCompiler;

	class SequenceOptimization
	{
	protected:
		SequenceOptimization(CodeOptimizationType optimizationType,
							 ModuleLogicCompiler& compiler,
							 CodeSnippet& srcCode);		// on input srcCode contains source code for optimization
														// after call optimize() srcCode contains optimized code
		virtual void reinitVars() = 0;
		virtual bool canStartSequence(const CodeItem& cmd) = 0;
		virtual bool isSequenceContinue(const CodeItem& cmd) = 0;
		virtual void getReplacementCode(CodeSnippet& code) = 0;

	public:
		bool optimize();

	private:
		CodeOptimizationType m_optimizationType;
		ModuleLogicCompiler& m_compiler;
		CodeSnippet& m_srcCode;

		int m_commandsInSequence = 0;
		CodeSnippetConstIterator m_firstSequenceCmd;
		CodeSnippetConstIterator m_lastSequenceCmd;
	};

	class SequentialMovesOptimization : public SequenceOptimization
	{
	public:
		SequentialMovesOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode,
									const LmMemoryMap& memoryMap);
	private:
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual void getReplacementCode(CodeSnippet& code) override;

		bool isAppropriateMoveCmd(const CodeItem& cmd) const;
		bool inSequence() const;

	private:
		const LmMemoryMap& m_memoryMap;

		quint16 m_sequenceMoveSizeW = 0;
		quint16 m_sequenceStartSrcAddr = 0;
		quint16 m_sequenceStartDestAddr = 0;
	};
}
