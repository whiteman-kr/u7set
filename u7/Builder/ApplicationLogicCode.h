#pragma once

#include <QObject>
#include "../lib/Types.h"
#include "LmMemoryMap.h"

namespace Builder
{
	enum class LmCommandCode
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
		RDFBTS = 13,
		SETMEM = 14,
		MOVB = 15,
		NSTART = 16,
		APPSTART = 17,
		MOV32 = 18,
		MOVC32 = 19,
		WRFB32 = 20,
		RDFB32 = 21,
		WRFBC32 = 22,
		RDFBTS32 = 23,
		MOVCF = 24,
		PMOV = 25,
		PMOV32 = 26,
	};


	struct LmCommand
	{
		LmCommandCode code;
		int sizeW;
		const char* str;

		bool waitFbExecution;

		int readTime;
		int runTime;

		static bool isValidCode(int commandCode);
		static int getSizeW(int commandCode);
		static const char* getStr(int commandCode);
	};


	const int CALC_RUNTIME = 99999;

	const LmCommand LmCommands[] =
	{
		{	LmCommandCode::NoCommand,	0,	"NO_CMD",	false,	0,	0					},
		{	LmCommandCode::NOP,			1,	"NOP",		false,	5,	2					},
		{	LmCommandCode::START,		2,	"START",	true,	8,	6					},
		{	LmCommandCode::STOP,		1,	"STOP",		false,	5,	2					},
		{	LmCommandCode::MOV,			3,	"MOV",		false,	11,	CALC_RUNTIME		},
		{	LmCommandCode::MOVMEM,		4,	"MOVMEM",	false,	14,	CALC_RUNTIME		},
		{	LmCommandCode::MOVC,		3,	"MOVC",		false,	11, CALC_RUNTIME		},
		{	LmCommandCode::MOVBC,		4,	"MOVBC",	false,	14, CALC_RUNTIME		},
		{	LmCommandCode::WRFB,		3,	"WRFB",		false,	11,	12					},
		{	LmCommandCode::RDFB,		3,	"RDFB",		true,	11,	7					},
		{	LmCommandCode::WRFBC,		3,	"WRFBC",	false,	11,	9					},
		{	LmCommandCode::WRFBB,		4,	"WRFBB",	false,	14,	11					},
		{	LmCommandCode::RDFBB,		4,	"RDFBB",	true,	14,	CALC_RUNTIME		},
		{	LmCommandCode::RDFBTS,		3,	"RDFBTS",	true,	11,	4					},
		{	LmCommandCode::SETMEM,		4,	"SETMEM",	false,	14, CALC_RUNTIME		},
		{	LmCommandCode::MOVB,		4,	"MOVB",		false,	14,	CALC_RUNTIME		},
		{	LmCommandCode::NSTART,		3,	"NSTART",	true,	11,	CALC_RUNTIME		},
		{	LmCommandCode::APPSTART,	2,	"APPSTART",	false,	8,	2					},
		{	LmCommandCode::MOV32,		3,	"MOV32",	false,	11,	14					},
		{	LmCommandCode::MOVC32,		4,	"MOVC32",	false,	14, 8					},
		{	LmCommandCode::WRFB32,		3,	"WRFB32",	false,	11,	20					},
		{	LmCommandCode::RDFB32,		3,	"RDFB32",	true,	11,	14					},
		{	LmCommandCode::WRFBC32,		4,	"WRFBC32",	false,	14,	16					},
		{	LmCommandCode::RDFBTS32,	4,	"RDFBTS32",	true,	14,	6					},
		{	LmCommandCode::MOVCF,		3,	"MOVCF",	false,	11,	6					},
		{	LmCommandCode::PMOV,		3,	"PMOV",		false,	11,	CALC_RUNTIME		},
		{	LmCommandCode::PMOV32,		3,	"PMOV32",	false,	11,	14					},
	};

	const int LM_COMMAND_COUNT = sizeof(LmCommands) / sizeof(LmCommand);


	const quint16	MIN_FB_TYPE = 1,
					MAX_FB_TYPE = 64 - 1,

					MIN_FB_INSTANCE = 1,
					MAX_FB_INSTANCE = 1024 - 1,

					MIN_FB_PARAM_NO = 0,
					MAX_FB_PARAM_NO = 64 - 1,

					MAX_BIT_NO_16 = 16 - 1;

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

		void setNoCommand() { opCode.code = static_cast<int>(LmCommandCode::NoCommand); }

		bool isNoCommand() const { return opCode.code == TO_INT(LmCommandCode::NoCommand); }

		void setOpCode(LmCommandCode code);
		int getOpCodeInt() const { return opCode.code; }
		LmCommandCode getOpCode() const { return static_cast<LmCommandCode>(opCode.code); }

		void setFbType(quint16 fbType);
		quint16 getFbType() const { return opCode.fbType; }
		QString getFbTypeStr() const;

		void setFbInstance(quint16 fbInstance);
		quint16 getFbInstance() const { return param.fbInstance; }
		int getFbInstanceInt() { return int(param.fbInstance); }

		void setFbCaption(const QString& fbCaption) { m_fbCaption = fbCaption.toUpper(); }
		QString getFbCaption() const { return m_fbCaption; }

		void setFbParamNo(quint16 fbParamNo);
		quint16 getFbParamNo() const { return param.fbParamNo; }
		int getFbParamNoInt() const { return int(param.fbParamNo); }

		void setWord2(quint16 value) { word2 = value; }
		quint16 getWord2() const { return word2; }

		void setWord3(quint16 value) { word3 = value; }
		quint16 getWord3() const { return word3; }

		void setWord4(quint16 value) { word4 = value; }
		quint16 getWord4() const { return word4; }

		void setBitNo(quint16 bitNo);

		void setBitNo1(quint16 bitNo);
		quint16 getBitNo1() const { return bitNo.b1; }

		void setBitNo2(quint16 bitNo);
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
	private:
		QString m_comment;

	protected:
		QByteArray m_binCode;

	public:
		CodeItem();
		CodeItem(const CodeItem& codeItem);

		virtual ~CodeItem() {}

		virtual QString toString() = 0;
		virtual int sizeW()= 0;

		virtual bool isCommand() = 0;
		virtual bool isComment() = 0;

		virtual void generateBinCode(E::ByteOrder byteOrder) = 0;
		virtual const QByteArray& getBinCode() { return m_binCode; }

		void setComment(const QString& comment) { m_comment = comment; }
		QString getComment() { return m_comment; }
		void clearComment() { m_comment.clear(); }

		bool commentIsEmpty() { return m_comment.isEmpty(); }
	};


	class Comment : public CodeItem
	{
	public:
		Comment();
		Comment(const Comment& comment);
		Comment(const QString& comment);

		QString toString() override;
		int sizeW() override { return 0; }

		void generateBinCode(E::ByteOrder) override {}

		bool isCommand() override { return false; }
		bool isComment() override { return true; }
	};


	class Command : public CodeItem
	{
	public:
		Command();
		Command(const Command& cmd);

		void nop();
		void start(quint16 fbType, quint16 fbInstance, const QString& fbCaption, int fbRunTime);
		void stop();
		void mov(quint16 addrTo, quint16 addrFrom);
		void mov(Address16 addrTo, Address16 addrFrom);
		void movMem(quint16 addrTo, quint16 addrFrom, quint16 sizeW);
		void movMem(Address16 addrTo, Address16 addrFrom, quint16 sizeW);
		void movConst(quint16 addrTo, quint16 constVal);
		void movBitConst(quint16 addrTo, quint16 bitNo, quint16 constBit);
		void movBitConst(Address16 addr16, quint16 constBit);
		void writeFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption);
		void readFuncBlock(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption);
		void writeFuncBlockConst(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 constVal, const QString& fbCaption);
		void writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, quint16 bitNo, const QString& fbCaption);
		void writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, Address16 addrFrom, const QString& fbCaption);
		void readFuncBlockBit(quint16 addrTo, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption);
		void readFuncBlockBit(Address16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption);
		void readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue, const QString& fbCaption);
		void setMem(quint16 addr, quint16 constValue, quint16 sizeW);
		void movBit(quint16 addrTo, quint16 bitTo, quint16 addrFrom, quint16 bitFrom);
		void movBit(Address16 addrTo, Address16 addrFrom);
		void nstart(quint16 fbType, quint16 fbInstance, quint16 startCount, const QString& fbCaption, int fbRunTime);
		void appStart(quint16 appStartAddr);

		void mov32(quint16 addrTo, quint16 addrFrom);
		void mov32(Address16 addrTo, Address16 addrFrom);
		void movConstInt32(quint16 addrTo, qint32 constInt32);
		void movConstUInt32(quint16 addrTo, quint32 constUInt32);
		void movConstFloat(quint16 addrTo, float constFloat);
		void writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption);
		void writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, Address16 addrFrom, const QString& fbCaption);
		void readFuncBlock32(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption);
		void readFuncBlock32(Address16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption);
		void writeFuncBlockConstInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 constInt32, const QString& fbCaption);
		void writeFuncBlockConstFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float constFloat, const QString& fbCaption);
		void readFuncBlockTestInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 testInt32, const QString& fbCaption);
		void readFuncBlockTestFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float testFloat, const QString& fbCaption);

		void movConstIfFlag(quint16 addrTo, quint16 constVal);
		void prevMov(quint16 addrTo, quint16 addrFrom);
		void prevMov32(quint16 addrTo, quint16 addrFrom);

		// --

		void setAddress(int address) { m_address = address; }

		QString toString() override;
		int sizeW() override { return m_code.sizeW(); }

		bool isCommand() override { return true; }
		bool isComment() override { return false; }

		bool hasError() const { return m_result == false; }

		bool isNoCommand() const { return m_code.isNoCommand(); }

		bool isOpCode(LmCommandCode code) const { return m_code.getOpCode() == code; }

		quint16 getWord2() const { return m_code.getWord2(); }

		bool isValidCommand() { return m_code.getOpCode() != LmCommandCode::NoCommand; }

		void generateBinCode(E::ByteOrder byteOrder) override;

		QString getMnemoCode();
		QString getConstValueString();

		int address() const { return m_address; }

		bool getTimes(int prevCmdExecTime);

		int execTime() const { return m_execTime; }
		int waitTime() const { return m_waitTime; }
		int waitAndExecTime() const { return m_waitTime + m_execTime; }

		static void setMemoryMap(LmMemoryMap* memoryMap, IssueLogger* log);
		static void resetMemoryMap();

	private:
		static int startFbExec(quint16 fbType, int fbRuntime);
		static void decFbExecTime(int time);
		static int getFbRemainingExecTime(quint16 fbType);

		void initStaticMembers();

		QString getCodeWordStr(int wordNo);

		bool addressInBitMemory(int address);
		bool addressInWordMemory(int address);

		bool read16(int addrFrom);
		bool read32(int addrFrom);
		bool readArea(int addrFrom, int sizeW);

		bool write16(int addrTo);
		bool write32(int addrTo);
		bool writeArea(int addrTo, int sizeW);

	private:
		int m_address = -1;

		CommandCode m_code;

		int m_waitTime = 0;				// command wait-to-execution time
		int m_execTime = 0;				// command execution time

		int m_fbExecTime = 0;			// != 0 for commands START and NSTART only

		bool m_result = true;

		static QHash<int, const LmCommand*> m_lmCommands;
		static QHash<quint16, int> m_executedFb;				// fbType => remaining FB exec time

		// initialized by ApplicationLogicCode::setMemoryMap()
		//
		static LmMemoryMap* m_memoryMap;
		static IssueLogger* m_log;
	};

	typedef QVector<Command> Commands;

	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT

	private:
		QVector<CodeItem*> m_codeItems;

		int m_commandAddress = 0;

		E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	public:
		ApplicationLogicCode();
		~ApplicationLogicCode();

		void setMemoryMap(LmMemoryMap* lmMemory, IssueLogger* log);

		void append(const Command& cmd);
		void append(const Commands& commands);
		void append(const Comment& cmt);
		void comment(QString commentStr);
		void newLine();

		void replaceAt(int commandIndex, const Command &cmd);

		void clear();

		void generateBinCode();

		void getAsmCode(QStringList& asmCode);
		void getBinCode(QByteArray& byteArray);
		void getMifCode(QStringList& mifCode);

		void getAsmMetadataFields(QStringList& metadataFields, int* metadataVersion);
		void getAsmMetadata(std::vector<QVariantList>& metadata);

		bool getRunTimes(int& idrPhaseClockCount, int& alpPhaseClockCount);

		void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

		int commandAddress() const { return m_commandAddress; }
	};

}
