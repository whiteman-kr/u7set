#pragma once

#include "../CommonLib/Types.h"
#include "../UtilsLib/WUtils.h"
#include "../Lib/LmDescription.h"

#include "LmMemoryMap.h"

namespace Builder
{
	struct LmCommand
	{
		enum class Code
		{
			NoCommand = 0,
			NOP = 1,
			START = 2,
			STOP = 3,
			MOV = 4,
			MOVMEM = 5,
			MOVC = 6,
			MOVBC = 7,
			WRFB = 8,
			RDFB = 9,
			WRFBC = 10,
			WRFBB = 11,
			RDFBB = 12,
			RDFBCMP = 13,
			SETMEM = 14,
			MOVB = 15,
			NSTART = 16,
			APPSTART = 17,
			MOV32 = 18,
			MOVC32 = 19,
			WRFB32 = 20,
			RDFB32 = 21,
			WRFBC32 = 22,
			RDFBCMP32 = 23,
			MOVCMPF = 24,
			PMOV = 25,
			PMOV32 = 26,
			FILL = 27,
		};

		static const int CALC_RUNTIME = 99999;
		static const quint16 MAX_FB_TYPE = 64 - 1;

		static const quint16 MIN_FB_PARAM_NO = 0;
		static const quint16 MAX_FB_PARAM_NO = 64 - 1;

		static const quint16 MAX_BIT_NO_16 = 16 - 1;

		LmCommand::Code code;
		int sizeW = 0;
		const char* mnemo = nullptr;

		bool waitFbExecution = false;

		int readTime = 0;
		int runTime = 0;
	};

	inline const LmCommand lmCommandSet[] =
	{
		{	LmCommand::Code::NoCommand,	0,	"NO_CMD",		false,	0,	0						},
		{	LmCommand::Code::NOP,		1,	"NOP",			false,	5,	2						},
		{	LmCommand::Code::START,		2,	"START",		true,	8,	6						},
		{	LmCommand::Code::STOP,		1,	"STOP",			false,	5,	2						},
		{	LmCommand::Code::MOV,		3,	"MOV",			false,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVMEM,	4,	"MOVMEM",		false,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVC,		3,	"MOVC",			false,	11, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVBC,		4,	"MOVBC",		false,	14, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::WRFB,		3,	"WRFB",			false,	11,	13						},
		{	LmCommand::Code::RDFB,		3,	"RDFB",			true,	11,	8						},
		{	LmCommand::Code::WRFBC,		3,	"WRFBC",		false,	11,	9						},
		{	LmCommand::Code::WRFBB,		4,	"WRFBB",		false,	14,	12						},
		{	LmCommand::Code::RDFBB,		4,	"RDFBB",		true,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::RDFBCMP,	3,	"RDFBCMP",		true,	11,	4						},
		{	LmCommand::Code::SETMEM,	4,	"SETMEM",		false,	14, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVB,		4,	"MOVB",			false,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::NSTART,	3,	"NSTART",		true,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::APPSTART,	2,	"APPSTART",		false,	8,	2						},
		{	LmCommand::Code::MOV32,		3,	"MOV32",		false,	11,	16						},
		{	LmCommand::Code::MOVC32,	4,	"MOVC32",		false,	14, 9						},
		{	LmCommand::Code::WRFB32,	3,	"WRFB32",		false,	11,	22						},
		{	LmCommand::Code::RDFB32,	3,	"RDFB32",		true,	11,	15						},
		{	LmCommand::Code::WRFBC32,	4,	"WRFBC32",		false,	14,	17						},
		{	LmCommand::Code::RDFBCMP32,	4,	"RDFBCMP32",	true,	14,	6						},
		{	LmCommand::Code::MOVCMPF,	3,	"MOVCMPF",		false,	11,	6						},
		{	LmCommand::Code::PMOV,		3,	"PMOV",			false,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::PMOV32,	3,	"PMOV32",		false,	11,	16						},
		{	LmCommand::Code::FILL,		4,	"FILL",			false,	11,	LmCommand::CALC_RUNTIME	},
	};

	const int LM_COMMANDS_COUNT = sizeof(lmCommandSet) / sizeof(LmCommand);

	class LmCommands : public std::map<int, const LmCommand>
	{
	public:
		LmCommands();

		bool isValidCode(LmCommand::Code commandCode) const;
		bool isValidCode(int commandCode) const;

		int getSizeW(LmCommand::Code commandCode) const;
		int getSizeW(int commandCode) const;

		QString getMnemo(LmCommand::Code commandCode) const;
		QString getMnemo(int commandCode) const;
	};

	extern LmCommands lmCommands;

	class CommandCode
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

		QString m_fbCaption;

		union
		{
			qint32 int32Value;
			float floatValue;
			quint32 uint32Value;
		} m_const;

		E::DataFormat m_constDataFormat = E::DataFormat::Float;

	public:
		CommandCode();
		CommandCode(const CommandCode& cCode);

		CommandCode& operator = (const CommandCode& cCode);

		void setNoCommand() { opCode.code = static_cast<int>(LmCommand::Code::NoCommand); }
		bool isNoCommand() const { return opCode.code == TO_INT(LmCommand::Code::NoCommand); }

		void setOpCode(LmCommand::Code code);
		int getOpCodeInt() const { return opCode.code; }
		LmCommand::Code getOpCode() const { return static_cast<LmCommand::Code>(opCode.code); }

		void setFbType(int fbType);
		quint16 getFbType() const { return opCode.fbType; }

		void setFbInstance(int fbInstance);
		quint16 getFbInstance() const { return param.fbInstance; }
		int getFbInstanceInt() const { return int(param.fbInstance); }

		void setFbCaption(const QString& fbCaption) { m_fbCaption = fbCaption.toUpper(); }
		QString getFbCaption() const { return m_fbCaption; }

		void setFbParamNo(int fbParamNo);
		quint16 getFbParamNo() const { return param.fbParamNo; }
		int getFbParamNoInt() const { return int(param.fbParamNo); }

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

		void setConstFloat(float floatValue);
		float getConstFloat() const;

		void setConstInt32(qint32 int32Value);
		qint32 getConstInt32() const;

		void setConstUInt32(quint32 uint32Value);
		quint32 getConstUInt32() const;

		E::DataFormat constDataFormat() const { return m_constDataFormat; }

		int sizeW() const;

		void calcCrc5();

		void clear();
	};

	struct CommandStatistics
	{
		LmCommand::Code code;

		int usedCount = 0;
		int codeSizeW = 0;
		int execTime = 0;					// waitTime + execTime

		CommandStatistics() = delete;
		CommandStatistics(LmCommand::Code cd) { code = cd; }
	};

	class CodeItem
	{
	public:
		CodeItem();

		void nop();
		void start(int fbType, int fbInstance, const QString& fbCaption, int fbRunTime);
		void stop();
		void mov(int addrTo, int addrFrom);
		void mov(Address16 addrTo, Address16 addrFrom);
		void movMem(int addrTo, int addrFrom, int sizeW);
		void movMem(Address16 addrTo, Address16 addrFrom, int sizeW);
		void movConst(int addrTo, int constVal);
		void movConst(Address16 addrTo, int constVal);
		void movBitConst(int addrTo, int bitNo, int constBit);
		void movBitConst(Address16 addr16, int constBit);
		void writeFuncBlock(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption);
		void writeFuncBlock(int fbType, int fbInstance, int fbParamNo, const Address16& addrFrom, const QString& fbCaption);
		void readFuncBlock(int addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlock(const Address16& addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal, const QString& fbCaption);
		void writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo, const QString& fbCaption);
		void writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom, const QString& fbCaption);
		void readFuncBlockBit(int addrTo, int bitNo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlockBit(Address16 addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlockCompare(int fbType, int fbInstance, int fbParamNo, int testValue, const QString& fbCaption);
		void setMem(int addr, int constValue, int sizeW);
		void setMem(Address16 addr, int constValue, int sizeW);
		void movBit(int addrTo, int bitTo, int addrFrom, int bitFrom);
		void movBit(Address16 addrTo, Address16 addrFrom);
		void nstart(int fbType, int fbInstance, int startCount, const QString& fbCaption, int fbRunTime);

		void appStart(int appStartAddr);

		void mov32(int addrTo, int addrFrom);
		void mov32(Address16 addrTo, Address16 addrFrom);
		void movConstInt32(int addrTo, qint32 constInt32);
		void movConstUInt32(int addrTo, quint32 constUInt32);
		void movConstUInt32(Address16 addrTo, quint32 constUInt32);
		void movConstFloat(int addrTo, float constFloat);
		void writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption);
		void writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom, const QString& fbCaption);
		void readFuncBlock32(int addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlock32(Address16 addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void writeFuncBlockConstInt32(int fbType, int fbInstance, int fbParamNo, qint32 constInt32, const QString& fbCaption);
		void writeFuncBlockConstFloat(int fbType, int fbInstance, int fbParamNo, float constFloat, const QString& fbCaption);
		void readFuncBlockCompareInt32(int fbType, int fbInstance, int fbParamNo, qint32 testInt32, const QString& fbCaption);
		void readFuncBlockCompareFloat(int fbType, int fbInstance, int fbParamNo, float testFloat, const QString& fbCaption);

		void movCompareFlag(int addrTo, int bitNo);
		void prevMov(int addrTo, int addrFrom);
		void prevMov32(int addrTo, int addrFrom);
		void fill(int addrTo, int addrFrom, int addrBit);
		void fill(Address16 addrTo, Address16 addrFrom);

		LmCommand::Code getOpcode() const { return m_code.getOpCode(); }

		int address() const { assert(m_isCommand == true); return m_address; }
		void setAddress(int address) { m_address = address; }

		QString comment() const { return m_comment; }
		void setComment(const QString& comment) { m_comment = comment; }
		void clearComment() { m_comment.clear(); }

		void setClockCount(int clockCount) { m_clockCount = clockCount; }

		int sizeW() const { return m_code.sizeW(); }

		bool isCommand() const { return m_isCommand == true; }
		bool isComment() const { return m_isCommand == false; }
		bool isNewLine() const { return m_isCommand == false && m_comment.isEmpty(); }

		bool hasError() const { return m_result == false; }

		bool isNoCommand() const { return m_code.isNoCommand(); }

		bool isOpCode(LmCommand::Code code) const { return m_code.getOpCode() == code; }
		bool isWaitingForFbExecution() const;

		quint16 getWord2() const { return m_code.getWord2(); }
		quint16 getWord3() const { return m_code.getWord3(); }

		quint16 getBitNo1() const { return m_code.getBitNo1(); }
		quint16 getBitNo2() const { return m_code.getBitNo2(); }

		int getFbType() const { return m_code.getFbType(); }
		int getFbInstance() const { return m_code.getFbInstance(); }

		bool isValidCommand() const { return m_code.getOpCode() != LmCommand::Code::NoCommand; }

		bool generateBinCode(QByteArray* binCode) const;

		QString getAsmCode(bool printCmdCode) const;
		QString mnemoCode() const;
		QString getConstValueString() const;

		bool calcRunTime(const LmDescription& lmDesc,
						 int prevCmdExecTime,
						 int* waitTime,
						 int* execTime,
						 int* fbExecTime);

		bool getTimes(int* waitTime, int* execTime) const;

		int waitTime() const { Q_ASSERT(m_waitTime != -1); return m_waitTime; }
		int execTime() const { Q_ASSERT(m_execTime != -1); return m_execTime; }

		void addExecTime(int execTime);

	private:
		void initCommand();

		QString getCodeWordStr(int wordNo) const;

		bool read16(int addrFrom);
		bool read32(int addrFrom);
		bool readArea(int addrFrom, int sizeW);

		bool write16(int addrTo);
		bool write32(int addrTo);
		bool writeArea(int addrTo, int sizeW);

		bool isAddrInBitMem(const LmDescription& lmDesc, quint32 addr) const;
		bool isAddrInWordMem(const LmDescription& lmDesc, quint32 addr) const;

	private:
		bool m_isCommand = false;

		int m_address = -1;
		CommandCode m_code;
		QString m_comment;

		int m_fbExecTime = 0;			// != 0 for commands START and NSTART only

		bool m_result = true;

		int m_waitTime = -1;
		int m_execTime = -1;
		int m_clockCount = -1;			// total code execution time after this command running
	};

	class CodeSnippet
	{
	public:
		CodeSnippet();

		// code snippet modification methods
		//
		void append(const CodeItem& codeItem);
		void append(const CodeSnippet& codeShippet);

		CodeSnippet& operator << (const CodeItem& ci);
		CodeSnippet& operator << (const CodeSnippet& codeShippet);

		void comment(const QString& cmt);
		void newLine();
		void comment_nl(const QString& cmt);
		void finalizeByNewLine();
		void clear();
		void reserve(int size);

		//

		bool isEmpty() const;
		int itemsCount() const;

		//

		void getAsmCode(QStringList* asmCode) const;
		void getBinCode(QByteArray* binCode) const;
		void getMifCode(QStringList* mifCode) const;

		void getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const;
		void getAsmMetadata(std::vector<QVariantList>* metadata) const;

		const std::vector<CodeItem>& code() const;

	protected:
		std::vector<CodeItem> m_code;
	};

	class AppLogicCode : public CodeSnippet
	{
	public:
		enum class Type
		{
			Unknown,
			IDR_Code,
			ALP_Code,
			AllCode
		};

	public:
		AppLogicCode(Type type);

		void setAppStartAddr(int addr);

		[[nodiscard]] bool finalize(std::shared_ptr<const LmDescription> lmDesc);

		void clear();

		Type codeType() const;

		int codeSizeW() const;
		int clockCount() const;
		int commandsCount() const;

		double lmCodeMemoryUsage() const;
		double execTimeMcs() const;
		double lmCycleTimeUsage() const;

		bool getCommandsStatistics(std::vector<CommandStatistics>* stat) const;

	private:
		int startFbExec(int fbOpCode, int fbRuntime);
		void decFbExecTime(int time);
		int getFbRemainingExecTime(int fbOpCode);
		int getMaxFbRemainingExecTimeAndClear();

	private:
		Type m_codeType = Type::Unknown;

		std::map<int, int> m_runningAfbs;		// AFB opCode -> AFB runtime

		int m_codeSizeW = -1;
		int m_commandsCount = -1;
		int m_clockCount = -1;

		double m_lmCodeMemUsage = 0;
		double m_execTimeMcs = 0;
		double m_lmCycleTimeUsage = 0;
	};

	struct CodeSnippetMetrics
	{
		void setStartAddr(int startAddr) { m_startAddr = startAddr; }
		void setEndAddr(int endAddr);

		double codePercent() const { return m_codePercent; }
		QString codePercentStr() const;

	private:
		int m_startAddr = 0;
		int m_endAddr = 0;
		int m_runTime = 0;

		double m_codePercent = 0;
	};
}
