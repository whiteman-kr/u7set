#pragma once

#include "../UtilsLib/WUtils.h"
#include <HardwareLib/LmDescription.h>

#include "LmMemoryMap.h"

namespace Builder
{
	class BinCommandCode
	{
	private:

#pragma pack(push, 1)
		union
		{
			struct
			{
				quint16 fbType : 6;
				quint16 code : 5;
				quint16 CRC5 : 5;
			} opCode;

			quint16 word1 = 0;
		};

		union
		{
			struct
			{
				quint16 fbParamNo : 6;
				quint16 fbInstance : 10;
			} param;

			quint16 word2 = 0;
		};

		quint16 word3 = 0;

		union
		{
			struct
			{
				quint8 b1;
				quint8 b2;
			} bitNo;

			quint16 word4 = 0;
		};

#pragma pack(pop)

	public:
		static const quint16 MAX_FB_TYPE = 64 - 1;
		static const quint16 MAX_FB_PARAM_NO = 64 - 1;
		static const quint16 MAX_BIT_NO_16 = 16 - 1;

	public:
		BinCommandCode();
		BinCommandCode(const BinCommandCode& cCode);

		BinCommandCode& operator = (const BinCommandCode& cCode);

		void setNoCommand() { word1 = LmCommand::NO_COMMAND; }
		bool isNoCommand() const { return word1 == LmCommand::NO_COMMAND; }

		void setOpCode(LmCommandCode code, quint16 cmdCodeMask);
//		LmCommandCode getOpCode() const { return static_cast<LmCommandCode>(opCode.code); }

		void setFbType(int fbType);
		quint16 getFbType() const { return opCode.fbType; }

		void setFbInstance(int fbInstance);
		quint16 getFbInstance() const { return param.fbInstance; }
		int getFbInstanceInt() const { return int(param.fbInstance); }

		void setFbParamNo(int fbParamNo);
		quint16 getFbParamNo() const { return param.fbParamNo; }
		int getFbParamNoInt() const { return int(param.fbParamNo); }

		quint16 getWord1BitNo() const { return word1 & 0x0F; }

		void setWord1(quint16 value) { word1 = value; }
		void setWord1(int value) { word1 = CHECK_AND_CAST_TO_QUINT16(value); }
		quint16 getWord1() const { return word1; }

		void setWord2(quint16 value) { word2 = value; }
		void setWord2(int value) { word2 = CHECK_AND_CAST_TO_QUINT16(value); }
		quint16 getWord2() const { return word2; }

		void setWord3(quint16 value) { word3 = value; }
		void setWord3(int value) { word3 = CHECK_AND_CAST_TO_QUINT16(value); }
		quint16 getWord3() const { return word3; }

		void setWord4(quint16 value) { word4 = value; }
		void setWord4(int value) { word4 = CHECK_AND_CAST_TO_QUINT16(value); }
		quint16 getWord4() const { return word4; }

		void setBitNo(int bitNo);

		void setBitNo1(int bitNo);
		quint16 getBitNo1() const { return bitNo.b1; }

		void setBitNo2(int bitNo);
		quint16 getBitNo2() const { return bitNo.b2; }

		quint16 getWord(int index) const;

		void calcCrc5();

		void clear();
	};

	struct CommandStatistics
	{
		LmCommandCode code;

		int usedCount = 0;
		int codeSizeW = 0;
		int execTime = 0;					// waitTime + execTime

		CommandStatistics() = delete;
		CommandStatistics(LmCommandCode cd) { code = cd; }
	};

	class CodeItem
	{
	public:
		CodeItem();

		CodeItem& startafb(int fbType, int fbInstance, const QString& fbCaption, int fbRunTime,
				   const QString& comment = Separator::EMPTY_STR);
		CodeItem& stop(const QString& comment = Separator::EMPTY_STR);
		CodeItem& mov(int addrTo, int addrFrom, const QString& comment = Separator::EMPTY_STR);
		CodeItem& mov(Address16 addrTo, Address16 addrFrom,
					  const QString& comment = Separator::EMPTY_STR);

		CodeItem& movAddrAcc(int addrTo, const QString& comment = Separator::EMPTY_STR);
		CodeItem& movAddrAcc(Address16 addrTo, const QString& comment = Separator::EMPTY_STR);

		CodeItem& movAccAddr(int addrFrom, const QString& comment = Separator::EMPTY_STR);
		CodeItem& movAccAddr(Address16 addrFrom, const QString& comment = Separator::EMPTY_STR);

		CodeItem& movMem(int addrTo, int addrFrom, int sizeW,
						 const QString& comment = Separator::EMPTY_STR);
		CodeItem& movMem(Address16 addrTo, Address16 addrFrom, int sizeW,
						 const QString& comment = Separator::EMPTY_STR);

		CodeItem& movConst(int addrTo, int constVal,
						   const QString& comment = Separator::EMPTY_STR);

		CodeItem& movConst(Address16 addrTo, int constVal,
						   const QString& comment = Separator::EMPTY_STR);

		CodeItem& movAccConst(int constVal, const QString& comment = Separator::EMPTY_STR);

		CodeItem& movBitConst(int addrTo, int bitNo, int constBit,
							  const QString& comment = Separator::EMPTY_STR);
		CodeItem& movBitConst(Address16 addr16, int constBit,
							  const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlock(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption,
								 const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlock(int fbType, int fbInstance, int fbParamNo, const Address16& addrFrom, const QString& fbCaption,
								 const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlock(int addrTo, int fbType, int fbInstance, int fbParamNo,
								const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlock(const Address16& addrTo, int fbType, int fbInstance, int fbParamNo,
								const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal,
									  const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo,
									const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom,
									const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlockBit(int addrTo, int bitNo, int fbType, int fbInstance, int fbParamNo,
								   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlockBit(Address16 addrTo, int fbType, int fbInstance, int fbParamNo,
								   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlockCompare(int fbType, int fbInstance, int fbParamNo, int testValue,
									   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& setMem(int addr, int constValue, int sizeW,
						 const QString& comment = Separator::EMPTY_STR);
		CodeItem& setMem(Address16 addr, int constValue, int sizeW,
						 const QString& comment = Separator::EMPTY_STR);

		CodeItem& movBit(int addrTo, int bitTo, int addrFrom, int bitFrom,
						 const QString& comment = Separator::EMPTY_STR);
		CodeItem& movBit(Address16 addrTo, Address16 addrFrom,
						 const QString& comment = Separator::EMPTY_STR);

		CodeItem& movBitAccAddr(int addrFrom, int bitNo, const QString& comment = Separator::EMPTY_STR);
		CodeItem& movBitAccAddr(Address16 addrFrom, const QString& comment = Separator::EMPTY_STR);

		CodeItem& movBitAddrAcc(int addrTo, int bitNo, const QString& comment = Separator::EMPTY_STR);
		CodeItem& movBitAddrAcc(Address16 addrTo, const QString& comment = Separator::EMPTY_STR);

		CodeItem& nstart(int fbType, int fbInstance, int startCount, const QString& fbCaption,
						 int fbRunTime, const QString& comment = Separator::EMPTY_STR);

		CodeItem& appStart(int appStartAddr, const QString& comment = Separator::EMPTY_STR);

		CodeItem& mov32(int addrTo, int addrFrom, const QString& comment = Separator::EMPTY_STR);
		CodeItem& mov32(Address16 addrTo, Address16 addrFrom,
						const QString& comment = Separator::EMPTY_STR);
		CodeItem& movConstInt32(int addrTo, qint32 constInt32,
								const QString& comment = Separator::EMPTY_STR);
		CodeItem& movConstUInt32(int addrTo, quint32 constUInt32,
								 const QString& comment = Separator::EMPTY_STR);
		CodeItem& movConstUInt32(Address16 addrTo, quint32 constUInt32,
								 const QString& comment = Separator::EMPTY_STR);
		CodeItem& movConstFloat(int addrTo, float constFloat, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, int addrFrom,
								   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom,
								   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlock32(int addrTo, int fbType, int fbInstance, int fbParamNo,
								  const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlock32(Address16 addrTo, int fbType, int fbInstance, int fbParamNo,
								  const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlockConstInt32(int fbType, int fbInstance, int fbParamNo, qint32 constInt32,
										   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& writeFuncBlockConstFloat(int fbType, int fbInstance, int fbParamNo, float constFloat,
										   const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlockCompareInt32(int fbType, int fbInstance, int fbParamNo, qint32 testInt32,
											const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);
		CodeItem& readFuncBlockCompareFloat(int fbType, int fbInstance, int fbParamNo, float testFloat,
											const QString& fbCaption, const QString& comment = Separator::EMPTY_STR);

		CodeItem& movCompareFlag(int addrTo, int bitNo,
								 const QString& comment = Separator::EMPTY_STR);
		CodeItem& prevMov(int addrTo, int addrFrom,
						  const QString& comment = Separator::EMPTY_STR);
		CodeItem& prevMov32(int addrTo, int addrFrom,
							const QString& comment = Separator::EMPTY_STR);
		CodeItem& fillb(int addrTo, int addrFrom, int addrBit,
					   const QString& comment = Separator::EMPTY_STR);
		CodeItem& fillb(Address16 addrTo, Address16 addrFrom,
					   const QString& comment = Separator::EMPTY_STR);

		CodeItem& resetAcc();
		CodeItem& setAcc();
		CodeItem& orAcc(const QString& comment = Separator::EMPTY_STR);
		CodeItem& andAcc(const QString& comment = Separator::EMPTY_STR);
		CodeItem& notAcc(const QString& comment = Separator::EMPTY_STR);
		CodeItem& lshift0Acc(const QString& comment = Separator::EMPTY_STR);
		CodeItem& lshift1Acc(const QString& comment = Separator::EMPTY_STR);

		E::DataFormat constDataFormat() const { return m_constDataFormat; }

		LmCommandCode lmCommandCode() const { return m_lmCmdCode; }

		bool setBinParams(LmDescriptionConstShared lmDesc, int address);

		int address() const;
		int sizeW() const;
		bool isWaitingForFbExecution() const;

		QString comment() const { return m_comment; }
		CodeItem& setComment(const QString& comment);
		void clearComment() { m_comment.clear(); }

		void setClockCount(int clockCount) { m_clockCount = clockCount; }

		bool isCommand() const { return m_isCommand == true; }
		bool isComment() const { return m_isCommand == false; }
		bool isNewLine() const { return m_isCommand == false && m_comment.isEmpty(); }

		bool hasError() const { return m_result == false; }

		bool isNoCommand() const { return m_code.isNoCommand(); }

		bool isOpCode(LmCommandCode code) const { return m_lmCmdCode == code; }

		quint16 getWord1() const { return m_code.getWord1(); }
		quint16 getWord2() const { return m_code.getWord2(); }
		quint16 getWord3() const { return m_code.getWord3(); }
		quint16 getWord4() const { return m_code.getWord4(); }

		quint16 getBitNo1() const { return m_code.getBitNo1(); }
		quint16 getBitNo2() const { return m_code.getBitNo2(); }
		quint16 getWord1BitNo() const { return m_code.getWord1BitNo(); }

		int getFbType() const { return m_code.getFbType(); }
		int getFbInstance() const { return m_code.getFbInstance(); }
		int getFbParamNo() const { return m_code.getFbParamNo(); }

		quint16 srcAddr() const;
		Address16 srcBitAddr() const;

		quint16 destAddr() const;
		Address16 destBitAddr() const;

		quint16 getMoveSizeW() const;

		quint16 getConst16() const;
		quint32 getConst32() const;
		quint16 getConstBit() const;

		bool isMoveCmd() const;
		bool isMove32Cmd() const;
		bool isMoveMemCmd() const;
		bool isMoveBitCmd() const;
		bool isMoveBitConstCmd() const;
		bool isMoveConstCmd() const;
		bool isMoveConst32Cmd() const;
		bool isSetMemCmd() const;
		bool isWriteFuncBlockBitCmd() const;
		bool isWriteFuncBlockConstCmd() const;
		bool isStartAfbCmd() const;
		bool isReadFuncBlockBitCmd() const;

		bool isValidCommand() const { return m_lmCmdCode != LmCommand::NO_COMMAND; }

		bool generateBinCode(QByteArray* binCode) const;

		QString getAsmCode(LmDescriptionConstShared lmDesc, bool printCmdCode, bool printTime) const;
		QString mnemoCode(LmDescriptionConstShared lmDesc) const;
		QString getConstValueString() const;

		bool calcRunTime(LmDescriptionConstShared lmDesc,
						 int prevCmdExecTime,
						 int waitFbTime,
						 int* waitTime,
						 int* execTime,
						 int* fbExecTime,
						 bool firstAlpCommand);

		bool getTimes(int* waitTime, int* execTime) const;

		int waitTime() const { Q_ASSERT(m_waitTime != -1); return m_waitTime; }
		int execTime() const { Q_ASSERT(m_execTime != -1); return m_execTime; }

		void addExecTime(int execTime);

	private:
		void initCommand();

		void setFbCaption(const QString& fbCaption) { m_fbCaption = fbCaption.toUpper(); }

		void setConstFloat(float floatValue);
		float getConstFloat() const;

		void setConstInt32(qint32 int32Value);
		qint32 getConstInt32() const;

		void setConstUInt32(quint32 uint32Value);
		quint32 getConstUInt32() const;

		QString getCodeWordStr(int wordNo) const;

		bool isAddrInBitMem(LmDescriptionConstShared lmDesc, quint32 addr) const;
		bool isAddrInWordMem(const LmDescription& lmDesc, quint32 addr) const;

		int calcRdFbRuntime(int cmdWaitTime,
							int preFbReadTime,
							int fbExecTime,
							int postFbReadTime) const;

	private:
		QString mnemo_nop() const;
		QString mnemo_acc() const;
		QString mnemo_startafb() const;
		QString mnemo_stop() const;
		QString mnemo_mov() const;
		QString mnemo_mov_addr_acc() const;
		QString mnemo_mov_acc_addr() const;
		QString mnemo_movmem() const;
		QString mnemo_movc() const;
		QString mnemo_movc_acc() const;
		QString mnemo_movbc() const;
		QString mnemo_wrfb() const;
		QString mnemo_rdfb() const;
		QString mnemo_wrfbc() const;
		QString mnemo_wrfbb() const;
		QString mnemo_rdfbb() const;
		QString mnemo_rdfbcmp() const;
		QString mnemo_setmem() const;
		QString mnemo_movb() const;
		QString mnemo_movb_acc_addr() const;
		QString mnemo_movb_addr_acc() const;
		QString mnemo_nstart() const;
		QString mnemo_appstart() const;
		QString mnemo_mov32() const;
		QString mnemo_movc32() const;
		QString mnemo_wrfb32() const;
		QString mnemo_rdfb32() const;
		QString mnemo_wrfbc32() const;
		QString mnemo_rdfbcmp32() const;
		QString mnemo_movcmpf() const;
		QString mnemo_pmov() const;
		QString mnemo_pmov32() const;
		QString mnemo_fillb() const;

		using GetMnemoFuncPtr = QString (CodeItem::*)() const;

		static inline const std::map<QString, GetMnemoFuncPtr> m_getMnemoFuncMap =	// getMnemoFuncName => getMnemoFuncPtr
		{
			{ QStringLiteral("mnemo_nop"), &CodeItem::mnemo_nop },
			{ QStringLiteral("mnemo_acc"), &CodeItem::mnemo_acc },
			{ QStringLiteral("mnemo_startafb"), &CodeItem::mnemo_startafb },
			{ QStringLiteral("mnemo_stop"), &CodeItem::mnemo_stop },
			{ QStringLiteral("mnemo_mov"), &CodeItem::mnemo_mov },
			{ QStringLiteral("mnemo_mov_addr_acc"), &CodeItem::mnemo_mov_addr_acc },
			{ QStringLiteral("mnemo_mov_acc_addr"), &CodeItem::mnemo_mov_acc_addr },
			{ QStringLiteral("mnemo_movmem"), &CodeItem::mnemo_movmem },
			{ QStringLiteral("mnemo_movc"), &CodeItem::mnemo_movc },
			{ QStringLiteral("mnemo_movc_acc"), &CodeItem::mnemo_movc_acc },
			{ QStringLiteral("mnemo_movbc"), &CodeItem::mnemo_movbc },
			{ QStringLiteral("mnemo_wrfb"), &CodeItem::mnemo_wrfb },
			{ QStringLiteral("mnemo_rdfb"), &CodeItem::mnemo_rdfb },
			{ QStringLiteral("mnemo_wrfbc"), &CodeItem::mnemo_wrfbc },
			{ QStringLiteral("mnemo_wrfbb"), &CodeItem::mnemo_wrfbb },
			{ QStringLiteral("mnemo_rdfbb"), &CodeItem::mnemo_rdfbb },
			{ QStringLiteral("mnemo_rdfbcmp"), &CodeItem::mnemo_rdfbcmp },
			{ QStringLiteral("mnemo_setmem"), &CodeItem::mnemo_setmem },
			{ QStringLiteral("mnemo_movb"), &CodeItem::mnemo_movb },
			{ QStringLiteral("mnemo_movb_acc_addr"), &CodeItem::mnemo_movb_acc_addr },
			{ QStringLiteral("mnemo_movb_addr_acc"), &CodeItem::mnemo_movb_addr_acc },
			{ QStringLiteral("mnemo_nstart"), &CodeItem::mnemo_nstart },
			{ QStringLiteral("mnemo_appstart"), &CodeItem::mnemo_appstart },
			{ QStringLiteral("mnemo_mov32"), &CodeItem::mnemo_mov32 },
			{ QStringLiteral("mnemo_movc32"), &CodeItem::mnemo_movc32 },
			{ QStringLiteral("mnemo_wrfb32"), &CodeItem::mnemo_wrfb32 },
			{ QStringLiteral("mnemo_rdfb32"), &CodeItem::mnemo_rdfb32 },
			{ QStringLiteral("mnemo_wrfbc32"), &CodeItem::mnemo_wrfbc32 },
			{ QStringLiteral("mnemo_rdfbcmp32"), &CodeItem::mnemo_rdfbcmp32 },
			{ QStringLiteral("mnemo_movcmpf"), &CodeItem::mnemo_movcmpf },
			{ QStringLiteral("mnemo_pmov"), &CodeItem::mnemo_pmov },
			{ QStringLiteral("mnemo_pmov32"), &CodeItem::mnemo_pmov32 },
			{ QStringLiteral("mnemo_fillb"), &CodeItem::mnemo_fillb }
		};

		// exectime_* function calculates and set:
		//		CodeItem::m_execTime
		//		CodeItem::m_waitTime
		//
		void exectime_const(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_startafb(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_nstart(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_write_bit_or_word_mem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_movmem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_setmem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_rdfb(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);
		void exectime_rdfb_bit(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);

		using CalcExecTimeFuncPtr = void (CodeItem::*)(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime);

		static inline const std::map<QString, CalcExecTimeFuncPtr> m_calcExecTimeFuncMap =	// calcExecTimeFuncName => calcExecTimeFuncPtr
		{
			{ QStringLiteral("exectime_const"), &CodeItem::exectime_const },
			{ QStringLiteral("exectime_startafb"), &CodeItem::exectime_startafb },
			{ QStringLiteral("exectime_nstart"), &CodeItem::exectime_nstart },
			{ QStringLiteral("exectime_write_bit_or_word_mem"), &CodeItem::exectime_write_bit_or_word_mem },
			{ QStringLiteral("exectime_movmem"), &CodeItem::exectime_movmem },
			{ QStringLiteral("exectime_setmem"), &CodeItem::exectime_setmem },
			{ QStringLiteral("exectime_rdfb"), &CodeItem::exectime_rdfb },
			{ QStringLiteral("exectime_rdfb_bit"), &CodeItem::exectime_rdfb_bit },
		};

	private:
		LmCommandCode m_lmCmdCode = LmCommand::NO_COMMAND;

		QString m_fbCaption;

		E::DataFormat m_constDataFormat = E::DataFormat::Float;

		union
		{
			qint32 int32Value;
			float floatValue;
			quint32 uint32Value;
		} m_const;


		bool m_isCommand = false;

		int m_address = -1;
		const LmCommand* m_lmCmd = nullptr;

		BinCommandCode m_code;
		QString m_comment;

		int m_fbExecTime = 0;			// != 0 for commands START and NSTART only

		bool m_result = true;

		int m_waitTime = -1;
		int m_execTime = -1;
		int m_clockCount = -1;			// total code execution time after this command running
	};
}
