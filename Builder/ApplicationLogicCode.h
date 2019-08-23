#pragma once

#include "../lib/Types.h"
#include "../lib/WUtils.h"

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
		static const quint16 MIN_FB_TYPE = 1;
		static const quint16 MAX_FB_TYPE = 64 - 1;

		static const quint16 MIN_FB_PARAM_NO = 0;
		static const quint16 MAX_FB_PARAM_NO = 64 - 1;

		static const quint16 MAX_BIT_NO_16 = 16 - 1;

		LmCommand::Code code;
		int sizeW;
		const char* mnemo;

		bool waitFbExecution;

		int readTime;
		int runTime;
	};

	const LmCommand lmCommandSet[] =
	{
		{	LmCommand::Code::NoCommand,	0,	"NO_CMD",		false,	0,	0						},
		{	LmCommand::Code::NOP,		1,	"NOP",			false,	5,	2						},
		{	LmCommand::Code::START,		2,	"START",		true,	8,	6						},
		{	LmCommand::Code::STOP,		1,	"STOP",			false,	5,	2						},
		{	LmCommand::Code::MOV,		3,	"MOV",			false,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVMEM,	4,	"MOVMEM",		false,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVC,		3,	"MOVC",			false,	11, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVBC,		4,	"MOVBC",		false,	14, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::WRFB,		3,	"WRFB",			false,	11,	12						},
		{	LmCommand::Code::RDFB,		3,	"RDFB",			true,	11,	7						},
		{	LmCommand::Code::WRFBC,		3,	"WRFBC",		false,	11,	9						},
		{	LmCommand::Code::WRFBB,		4,	"WRFBB",		false,	14,	11						},
		{	LmCommand::Code::RDFBB,		4,	"RDFBB",		true,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::RDFBCMP,	3,	"RDFBCMP",		true,	11,	4						},
		{	LmCommand::Code::SETMEM,	4,	"SETMEM",		false,	14, LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::MOVB,		4,	"MOVB",			false,	14,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::NSTART,	3,	"NSTART",		true,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::APPSTART,	2,	"APPSTART",		false,	8,	2						},
		{	LmCommand::Code::MOV32,		3,	"MOV32",		false,	11,	14						},
		{	LmCommand::Code::MOVC32,	4,	"MOVC32",		false,	14, 8						},
		{	LmCommand::Code::WRFB32,	3,	"WRFB32",		false,	11,	20						},
		{	LmCommand::Code::RDFB32,	3,	"RDFB32",		true,	11,	14						},
		{	LmCommand::Code::WRFBC32,	4,	"WRFBC32",		false,	14,	16						},
		{	LmCommand::Code::RDFBCMP32,	4,	"RDFBCMP32",	true,	14,	6						},
		{	LmCommand::Code::MOVCMPF,	3,	"MOVCMPF",		false,	11,	6						},
		{	LmCommand::Code::PMOV,		3,	"PMOV",			false,	11,	LmCommand::CALC_RUNTIME	},
		{	LmCommand::Code::PMOV32,	3,	"PMOV32",		false,	11,	14						},
		{	LmCommand::Code::FILL,		4,	"FILL",			false,	11,	14						},
	};

	class LmCommands : public QHash<int, LmCommand>
	{
	public:
		LmCommands();

		bool isValidCode(LmCommand::Code commandCode);
		bool isValidCode(int commandCode);

		int getSizeW(LmCommand::Code commandCode);
		int getSizeW(int commandCode);

		QString getMnemo(LmCommand::Code commandCode);
		QString getMnemo(int commandCode);
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
		void movBitConst(int addrTo, int bitNo, int constBit);
		void movBitConst(Address16 addr16, int constBit);
		void writeFuncBlock(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption);
		void readFuncBlock(int addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal, const QString& fbCaption);
		void writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo, const QString& fbCaption);
		void writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom, const QString& fbCaption);
		void readFuncBlockBit(int addrTo, int bitNo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlockBit(Address16 addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption);
		void readFuncBlockCompare(int fbType, int fbInstance, int fbParamNo, int testValue, const QString& fbCaption);
		void setMem(int addr, int constValue, int sizeW);
		void movBit(int addrTo, int bitTo, int addrFrom, int bitFrom);
		void movBit(Address16 addrTo, Address16 addrFrom);
		void nstart(int fbType, int fbInstance, int startCount, const QString& fbCaption, int fbRunTime);

		void appStart(int appStartAddr);

		void mov32(int addrTo, int addrFrom);
		void mov32(Address16 addrTo, Address16 addrFrom);
		void movConstInt32(int addrTo, qint32 constInt32);
		void movConstUInt32(int addrTo, quint32 constUInt32);
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

		//

		bool checkNop();
		bool checkStart();
		bool checkStop();
		bool checkMov();
		bool checkMovMem();
		bool checkMovConst();
		bool checkMovBitConst();
		bool checkWriteFuncBlock();
		bool checkReadFuncBlock();
		bool checkWriteFuncBlockConst();
		bool checkWriteFuncBlockBit();
		bool checkReadFuncBlockBit();
		bool checkReadFuncBlockTest();
		bool checkSetMem();
		bool checkMovBit();
		bool checkNstart();
		bool checkAppStart();
		bool checkMov32();
		bool checkMovConst32();
		bool checkWriteFuncBlock32();
		bool checkReadFuncBlock32();
		bool checkWriteFuncBlockConst32();
		bool checkReadFuncBlockTest32();
		bool checkMovConstIfFlag();
		bool checkPrevMov();
		bool checkPrevMov32();
		bool checkFill();

		//

		LmCommand::Code getOpcode() const { return m_code.getOpCode(); }

		int address() const { assert(m_isCommand == true); return m_address; }
		void setAddress(int address) { m_address = address; }

		QString comment() const { return m_comment; }
		void setComment(const QString& comment) { m_comment = comment; }
		void clearComment() { m_comment.clear(); }

		int sizeW() const { return m_code.sizeW(); }

		bool isCommand() const { return m_isCommand == true; }
		bool isComment() const { return m_isCommand == false; }
		bool isNewLine() const { return m_isCommand == false && m_comment.isEmpty(); }

		bool hasError() const { return m_result == false; }

		bool isNoCommand() const { return m_code.isNoCommand(); }

		bool isOpCode(LmCommand::Code code) const { return m_code.getOpCode() == code; }

		quint16 getWord2() const { return m_code.getWord2(); }
		quint16 getWord3() const { return m_code.getWord3(); }

		quint16 getBitNo1() const { return m_code.getBitNo1(); }
		quint16 getBitNo2() const { return m_code.getBitNo2(); }

		bool isValidCommand() const { return m_code.getOpCode() != LmCommand::Code::NoCommand; }

		bool generateBinCode(QByteArray* binCode) const;

		QString getAsmCode(bool printCmdCode) const;
		QString mnemoCode() const;
		QString getConstValueString() const;

		bool getTimes(const LmMemoryMap* lmMemMap, int prevCmdExecTime, int* waitTime, int* execTime) const;

	private:
		void initCommand();

		static int startFbExec(int fbType, int fbRuntime);
		static void decFbExecTime(int time);
		static int getFbRemainingExecTime(int fbType);

		QString getCodeWordStr(int wordNo) const;

		bool read16(int addrFrom);
		bool read32(int addrFrom);
		bool readArea(int addrFrom, int sizeW);

		bool write16(int addrTo);
		bool write32(int addrTo);
		bool writeArea(int addrTo, int sizeW);

	private:
		bool m_isCommand = false;

		int m_address = -1;
		CommandCode m_code;
		QString m_comment;

		qint32 m_numerator = 0;			// unique number of codeItems for debugging purposes

		int m_fbExecTime = 0;			// != 0 for commands START and NSTART only

		bool m_result = true;

		static QHash<int, int> m_executedFb;				// fbType => remaining FB exec time

		static qint32 m_codeItemsNumerator;
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

	class CodeSnippet : public QVector<CodeItem>
	{
	public:
		CodeSnippet();

		void comment(const QString& cmt);
		void newLine();
		void comment_nl(const QString& cmt);
		void finalizeByNewLine();

		void init(CodeSnippetMetrics* codeFragmentMetrics);
		void calculate(CodeSnippetMetrics* codeFragmentMetrics);

		int sizeW() const;
	};

	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicCode();
		~ApplicationLogicCode();

		void setMemoryMap(LmMemoryMap* lmMemory, IssueLogger* log);

		void append(const CodeItem& codeItem);
		void append(const CodeSnippet& codeShippet);
		void comment(const QString& str);
		void newLine();

		void clear();

		void getAsmCode(QStringList* asmCode) const;
		void getBinCode(QByteArray* binCode) const;
		void getMifCode(QStringList* mifCode) const;

		void getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const;
		void getAsmMetadata(std::vector<QVariantList>* metadata) const;

		bool getRunTimes(int* idrPhaseClockCount, int* alpPhaseClockCount) const;

		int commandAddress() const { return m_commandAddress; }

	private:
		QVector<CodeItem> m_codeItems;
		int m_commandAddress = 0;

		LmMemoryMap* m_lmMemoryMap = nullptr;
		IssueLogger* m_log = nullptr;
	};

}
