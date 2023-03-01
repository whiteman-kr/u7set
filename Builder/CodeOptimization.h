#pragma once

#include "ApplicationLogicCode.h"

namespace Builder
{
	enum class CodeOptimizationType
	{
		None,
		SequentialMoves,
		SequentialConstMoves,
		SequentialBitMoves,
		BitFilling,
	};

	struct OptimizationInfo
	{
		OptimizationInfo() = delete;

		OptimizationInfo(CodeOptimizationType t) : type(t) { }

		CodeOptimizationType type = CodeOptimizationType::None;
		int optimizationsCount = 0;
		int codeReductionSizeW = 0;

		static QString typeStr(CodeOptimizationType t);
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
		virtual bool canOptimize() const = 0;

		const ModuleLogicCompiler& compiler() const;

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
		virtual bool canOptimize() const override;
		virtual void getReplacementCode(CodeSnippet& code) override;

		bool isAppropriateMoveCmd(const CodeItem& cmd) const;
		bool inSequence() const;

	private:
		const LmMemoryMap& m_memoryMap;

		quint16 m_sequenceMoveSizeW = 0;
		quint16 m_sequenceStartSrcAddr = 0;
		quint16 m_sequenceStartDestAddr = 0;
	};

	class SequentialConstMovesOptimization : public SequenceOptimization
	{
	public:
		SequentialConstMovesOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode,
									const LmMemoryMap& memoryMap);
	private:
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual void getReplacementCode(CodeSnippet& code) override;

		bool isAppropriateMoveCmd(const CodeItem& cmd) const;
		bool inSequence() const;

	private:
		const LmMemoryMap& m_memoryMap;

		quint16 m_moveConst = 0;
		quint16 m_sequenceMoveSizeW = 0;
		quint16 m_sequenceStartDestAddr = 0;
	};

	class SequentialBitMovesOptimization : public SequenceOptimization
	{
	public:
		SequentialBitMovesOptimization(ModuleLogicCompiler& compiler,
										CodeSnippet& srcCode);
	private:
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual void getReplacementCode(CodeSnippet& code) override;

		bool setBit(int bitNo);
		bool inSequence() const;

	private:
		int m_srcAddr = 0;
		int m_destAddr = 0;

		quint16 m_bitField = 0;
		int m_bitCount = 0;

		int m_directMoveDestAddr = BAD_ADDRESS;

		int m_bitAccAddr = 0;
	};

	class BitFillingOptimization : public SequenceOptimization
	{
	public:
		BitFillingOptimization(ModuleLogicCompiler& compiler,
										CodeSnippet& srcCode);
	private:
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual void getReplacementCode(CodeSnippet& code) override;

		bool setBit(int bitNo);
		bool inSequence() const;

	private:
		Address16 m_srcBitAddr;
		int m_destAddr = 0;

		quint16 m_bitField = 0;
		int m_bitCount = 0;

		int m_directMoveDestAddr = BAD_ADDRESS;

		int m_bitAccAddr = 0;
	};

}

