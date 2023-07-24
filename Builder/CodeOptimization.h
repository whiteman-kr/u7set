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
		BitFilling,
		BitAccNot,
		SequentialAccBitMoves,
		BitAccAnd,
		BitAccOr,
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

	class BitAccNotOptimization : public SequenceOptimization
	{
		//
		// Replace AFB based NOT operation:
		//
		//	WRFBB     NOT.0[0], 46083[0]
		//	STARTAFB  NOT.0
		//	RDFBB     46084[0], NOT.0[2]
		//
		// with bit ACC based commands:
		//
		//	MOVB	  ACC, 46083[0]
		//	NOT		  ACC
		//  MOVB	  46084[1], ACC
		//

	public:
		BitAccNotOptimization(ModuleLogicCompiler& compiler,
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
		Address16 m_destBitAddr;

		int m_sequenceState = -1;					// -1 not in sequence
													// 0 bit loaded to AFB NOT
													// 1 AFB NOT started
													// 2 result read from AFB NOT and saved
		static const int NOT_AFB_OPCODE = 2;
		static const int AFB_NOT_IN_PIN_INDEX = 0;
		static const int AFB_NOT_OUT_PIN_INDEX = 2;
	};

	class SequentialAccBitMovesOptimization : public SequenceOptimization
	{
		// Replace sequential bit mov from different words to another word:
		//
		//		MOVB      46080[0], 56382[15]
		//		MOVB      46080[1], 56383[0]
		//		MOVB      46080[2], 56383[1]
		//		MOVB      46080[3], 56383[2]
		//		MOVB      46080[4], 56384[0]
		//		MOVB      46080[5], 56384[1]
		//		MOVBC     46080[6], #1
		//		MOVB      46080[7], 56384[3]
		//		MOVB      46080[8], 56384[4]
		//		MOVB      46080[9], 56384[5]
		//		MOVBC     46080[10], #0
		//		MOVB      46080[11], 56384[7]
		//		MOVB      46080[12], 56384[8]
		//		MOVB      46080[13], 56384[9]
		//		MOVB      46080[14], 56398[0]
		//		MOVB      46080[15], 56398[1]
		//		MOV       56350, 46080
		//
		// by bit ACC using commands (reverse order bit loading!):
		//
		//		MOVB      ACC, 56398[1]
		//		MOVB      ACC, 56398[0]
		//		MOVB      ACC, 56384[9]
		//		MOVB      ACC, 56384[8]
		//		MOVB      ACC, 56384[7]
		//		LSHIFT0	  ACC
		//		MOVB      ACC, 56384[5]
		//		MOVB      ACC, 56384[4]
		//		MOVB      ACC, 56384[3]
		//		LSHIFT1	  ACC
		//		MOVB      ACC, 56384[1]
		//		MOVB      ACC, 56384[0]
		//		MOVB      ACC, 56383[2]
		//		MOVB      ACC, 56383[1]
		//		MOVB      ACC, 56383[0]
		//		MOVB      ACC, 56382[15]
		//		MOV       56350, ACC

	public:
		SequentialAccBitMovesOptimization(ModuleLogicCompiler& compiler,
										CodeSnippet& srcCode);
	private:
		virtual bool isOptimizationPossible() const override;
		virtual void reinitVars() override;
		virtual bool canStartSequence(const CodeItem& cmd) override;
		virtual bool isSequenceContinue(const CodeItem& cmd) override;
		virtual bool canOptimize() const override;
		virtual bool getReplacementCode(CodeSnippet& code) override;

		bool inSequence() const;

	private:
		mutable Address16 m_constBit0Addr;
		mutable Address16 m_constBit1Addr;

		int m_sequenceState = -1;			// -1	not in sequence
											// 0	pass load const 0 to accumulator
											// 1	loading bits to accumulator
											// 2	pass move from accumulator to mem
		int m_prevDestAccAddr = BAD_ADDRESS;
		int m_destAccAddr = BAD_ADDRESS;

		Address16 m_bitSrcAddrs[16];
		bool m_destAccZeroInitPresent = false;

		int m_directMoveDestAddr = BAD_ADDRESS;
	};

	class BitAccAndOptimization : public SequenceOptimization
	{
		//
		// Replace AFB based AND operation:
		//
		//	WRFBB     AND.0[3], 46084[0]
		//	WRFBB     AND.0[4], 46084[1]
		//	STARTAFB  AND.0
		//	RDFBB     46083[2], AND.0[20]
		//
		// with bit ACC based commands:
		//
		//	SET		  ACC
		//	MOVB	  ACC, 46084[0]
		//	MOVB	  ACC, 46084[1]
		//	AND		  ACC
		//  MOVB	  46083[2], ACC
		//

	public:
		BitAccAndOptimization(ModuleLogicCompiler& compiler,
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
		bool processCommand(const CodeItem& cmd);

	private:
		static const int LOGIC_AFB_OPCODE = 1;
		static const int LOGIC_CONF_AND = 1;

		//
		mutable Address16 m_constBit0Addr;
		mutable Address16 m_constBit1Addr;
		mutable std::map<int, int> m_afbInstances;		// instance => operand count
		mutable std::set<int> m_inputIndexes;
		mutable int m_outputIndex = -1;

		//

		int m_sequenceState = -1;					// -1 not in sequence
													// 0 loading source bits in AFB
													// 1 AFB AND started
													// 2 result read from AFB and saved
		int m_loadBitCount = 0;
		int m_afbInstance = -1;
		std::set<Address16> m_srcBitAddrs;
		Address16 m_destBitAddr;
	};

	class BitAccOrOptimization : public SequenceOptimization
	{
		//
		// Replace AFB based OR operation:
		//
		//	WRFBB     OR.0[3], 46084[0]
		//	WRFBB     OR.0[4], 46084[1]
		//	STARTAFB  OR.0
		//	RDFBB     46083[2], OR.0[20]
		//
		// with bit ACC based commands:
		//
		//	RESET		  ACC
		//	MOVB	  ACC, 46084[0]
		//	MOVB	  ACC, 46084[1]
		//	OR		  ACC
		//  MOVB	  46083[2], ACC
		//

	public:
		BitAccOrOptimization(ModuleLogicCompiler& compiler,
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
		bool processCommand(const CodeItem& cmd);

	private:
		static const int LOGIC_AFB_OPCODE = 1;
		static const int LOGIC_CONF_OR = 2;

		//

		mutable Address16 m_constBit0Addr;
		mutable Address16 m_constBit1Addr;
		mutable std::map<int, int> m_afbInstances;		// instance => operand count
		mutable std::set<int> m_inputIndexes;
		mutable int m_outputIndex = -1;

		//

		int m_sequenceState = -1;					// -1 not in sequence
													// 0 loading source bits in AFB
													// 1 AFB AND started
													// 2 result read from AFB and saved
		int m_loadBitCount = 0;
		int m_afbInstance = -1;
		std::set<Address16> m_srcBitAddrs;
		Address16 m_destBitAddr;
	};
}

