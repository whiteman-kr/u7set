#pragma once

#include "AppLogicCode.h"

namespace Builder
{
	enum class CodeOptimizationType
	{
		None,
		SequentialMoves,
		SequentialConstMoves,
		SequentialBitMoves,
		BitFilling
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

		virtual bool isOptimizationPossible() const = 0;

		virtual void reinitVars() = 0;
		virtual bool canStartSequence(const CodeItem& cmd) = 0;
		virtual bool isSequenceContinue(const CodeItem& cmd) = 0;
		virtual bool getReplacementCode(CodeSnippet& code) = 0;
		virtual bool canOptimize() const = 0;

		const ModuleLogicCompiler& compiler() const;

		bool hasRequiredCommands(const std::vector<LmCommandCode>& requiredCmdCodes) const;

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
		virtual bool isOptimizationPossible() const override;
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual bool getReplacementCode(CodeSnippet& code) override;

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
		//
		// Replace sequential const loading
		//
		//		MOVC32    56055, #0
		//		MOVC32    56057, #0
		//		MOVC32    56059, #0
		//		MOVC32    56061, #0
		//		MOVC32    56063, #0
		//
		// by one command SETMEM
		//
		//		SETMEM    56055, #0, 10
		//

	public:
		SequentialConstMovesOptimization(ModuleLogicCompiler& compiler,
									CodeSnippet& srcCode,
									const LmMemoryMap& memoryMap);
	private:
		virtual bool isOptimizationPossible() const override;
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual bool getReplacementCode(CodeSnippet& code) override;

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
		// Replace sequential bit mov from one word to another (bit to bit):
		//
		//		MOVB      46080[0], 56375[0]
		//		MOVB      46080[1], 56375[1]
		//		MOVB      46080[2], 56375[2]
		//		MOVB      46080[3], 56375[3]
		//		MOVB      46080[4], 56375[4]
		//		MOVB      46080[5], 56375[5]
		//		MOVB      46080[6], 56375[6]
		//		MOVB      46080[7], 56375[7]
		//		MOVB      46080[8], 56375[8]
		//		MOVB      46080[9], 56375[9]
		//		MOVB      46080[10], 56375[10]
		//		MOVB      46080[12], 56375[12]
		//		MOVB      46080[13], 56375[13]
		//		MOVB      46080[14], 56375[14]
		//		MOVB      46080[15], 56375[15]
		//		MOV       56347, 46080
		//
		// By single mov word command:
		//
		//		MOV       56347, 56375

	public:
		SequentialBitMovesOptimization(ModuleLogicCompiler& compiler,
										CodeSnippet& srcCode);
	private:
		virtual bool isOptimizationPossible() const override;
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual bool getReplacementCode(CodeSnippet& code) override;

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
		// Replace one bit loading in all bits of word:
		//
		//		MOVB      55882[0], 46103[3]
		//		MOVB      55882[1], 46103[3]
		//		MOVB      55882[2], 46103[3]
		//		MOVB      55882[3], 46103[3]
		//		MOVB      55882[4], 46103[3]
		//		MOVB      55882[5], 46103[3]
		//		MOVB      55882[6], 46103[3]
		//		MOVB      55882[7], 46103[3]
		//		MOVB      55882[8], 46103[3]
		//		MOVB      55882[9], 46103[3]
		//		MOVB      55882[10], 46103[3]
		//		MOVB      55882[11], 46103[3]
		//		MOVB      55882[12], 46103[3]
		//		MOVB      55882[13], 46103[3]
		//		MOVB      55882[14], 46103[3]
		//		MOVB      55882[15], 46103[3]
		//
		// by one command FILLB:
		//
		//		FILLB     55882, 46103[3]
		//

	public:
		BitFillingOptimization(ModuleLogicCompiler& compiler,
										CodeSnippet& srcCode);
	private:
		virtual bool isOptimizationPossible() const override;
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual bool getReplacementCode(CodeSnippet& code) override;

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

