#pragma once

#include <QObject>
#include "../include/Types.h"

namespace Builder
{

	enum class CommandCodes
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

		Count
	};


	const CommandCodes AllCommandCodes[] =
	{
		CommandCodes::NOP,
		CommandCodes::START,
		CommandCodes::STOP,
		CommandCodes::MOV,
		CommandCodes::MOVMEM,
		CommandCodes::MOVC,
		CommandCodes::MOVBC,
		CommandCodes::WRFB,
		CommandCodes::RDFB,
		CommandCodes::WRFBC,
		CommandCodes::WRFBB,
		CommandCodes::RDFBB,
		CommandCodes::RDFBTS,
		CommandCodes::SETMEM,
		CommandCodes::MOVB,
	};


	const int COMMAND_COUNT = static_cast<int>(CommandCodes::Count);

	const int CommandLen[COMMAND_COUNT] =
	{
		0,		//	NoCommand
		1,		//	NOP
		2,		//	START
		1,		//  STOP
		3,		//	MOV
		4,		//	MOVMEM
		3,		//	MOVC
		4,		//	MOVBC
		3,		//	WRFB
		3,		//	RDFB
		3,		//	WRFBC
		4,		//	WRFBB
		4,		//	RDFBB
		3,		//	RDFBTS
		4,		//	SETMEM
		4,		//	MOVB
	};

	const char* const CommandStr[COMMAND_COUNT] =
	{
		"NO_CMD",
		"NOP",
		"START",
		"STOP",
		"MOV",
		"MOVMEM",
		"MOVC",
		"MOVBC",
		"WRFB",
		"RDFB",
		"WRFBC",
		"WRFBB",
		"RDFBB",
		"RDFBTS",
		"SETMEM",
		"MOVB",
	};


	enum class FbType
	{
		UNKNOWN = 0,
		AND,
		OR,
		XOR,
		NOT,
		TCT,
		SR_RS,
		CTUD,
		MAJ,
		SRSST,
		BCOD,
		BDEC,
		BCOMP,
		LAG,
		MID,
		ADD,
		SCAL,
		LINFUN,
		SQRT,
		SIN,
		COS,
		DIV,
		MULT,
		ABS,
		LN,
		LIM,
		MIN_MAX,
		PID,

		Count
	};

	const char* const FbTypeStr[] =
	{
		"???",
		"AND",
		"OR",
		"XOR",
		"NOT",
		"TCT",
		"SR/RS",
		"CTUD",
		"MAJ",
		"SRSST",
		"BCOD",
		"BDEC",
		"BCOMP",
		"LAG",
		"MID",
		"ADD",
		"SCAL",
		"LINFUN",
		"SQRT",
		"SIN",
		"COS",
		"DIV",
		"MULT",
		"ABS",
		"LN",
		"LIM",
		"MIN/MAX",
		"PID",
	};

	const int FB_TYPE_STR_COUNT = sizeof(FbTypeStr) / sizeof(const char*);


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

	public:
		CommandCode()
		{
			setNoCommand();
		}

		void setNoCommand() { opCode.code = static_cast<int>(CommandCodes::NoCommand); }

		void setOpCode(CommandCodes code);
		int getOpCodeInt() { return opCode.code; }
		CommandCodes getOpCode() { return static_cast<CommandCodes>(opCode.code); }

		void setFbType(quint16 fbType);
		quint16 getFbType() { return opCode.fbType; }
		QString getFbTypeStr();

		void setFbInstance(quint16 fbInstance);
		quint16 getFbInstance() { return param.fbInstance; }
		int getFbInstanceInt() { return int(param.fbInstance); }

		void setFbParamNo(quint16 fbParamNo);
		quint16 getFbParamNo() { return param.fbParamNo; }
		int getFbParamNoInt() { return int(param.fbParamNo); }

		void setWord2(quint16 value) { word2 = value; }
		quint16 getWord2() { return word2; }

		void setWord3(quint16 value) { word3 = value; }
		quint16 getWord3() { return word3; }

		void setWord4(quint16 value) { word4 = value; }
		quint16 getWord4() { return word4; }

		void setBitNo(quint16 bitNo);

		void setBitNo1(quint16 bitNo);
		quint16 getBitNo1() const { return bitNo.b1; }

		void setBitNo2(quint16 bitNo);
		quint16 getBitNo2() const { return bitNo.b2; }

		quint16 getWord(int index);

		int sizeW();
	};


	class CodeItem
	{
	private:
		QString m_comment;

	protected:
		QByteArray m_binCode;

	public:
		virtual ~CodeItem();

		virtual QString toString() = 0;
		virtual int sizeW()= 0;

		virtual bool isCommand() = 0;
		virtual bool isComment() = 0;

		virtual void generateBinCode(ByteOrder byteOrder) = 0;
		virtual const QByteArray& getBinCode() { return m_binCode; }

		void setComment(const QString& comment) { m_comment = comment; }
		QString getComment() { return m_comment; }

		bool commentIsEmpty() { return m_comment.isEmpty(); }
	};


	class Comment : public CodeItem
	{
	public:
		Comment() {}
		Comment(const QString& comment) { setComment(comment); }

		QString toString() override;
		int sizeW() override { return 0; }

		void generateBinCode(ByteOrder) override {}

		bool isCommand() override { return false; }
		bool isComment() override { return true; }
	};


	class Command : public CodeItem
	{
	private:
		int m_address = -1;

		CommandCode m_code;

		QString getCodeWordStr(int wordNo);

	public:
		Command() {}

		void nop();
		void start(quint16 fbType, quint16 fbInstance);
		void stop();
		void mov(quint16 addrTo, quint16 addrFrom);
		void movMem(quint16 addrTo, quint16 addrFrom, quint16 sizeW);
		void movConst(quint16 addrTo, quint16 constVal);
		void movBitConst(quint16 addrTo, quint16 bitNo, quint16 constBit);
		void writeFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom);
		void readFuncBlock(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void writeFuncBlockConst(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 constVal);
		void writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, quint16 bitNo);
		void readFuncBlockBit(quint16 addrTo, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue);
		void setMem(quint16 addr, quint16 sizeW, quint16 constValue);
		void moveBit(quint16 addrTo, quint16 addrToMask, quint16 addrFrom, quint16 addrFromMask);

		void setAddress(int address) { m_address = address; }

		QString toString() override;
		int sizeW() override { return m_code.sizeW(); }

		bool isCommand() override { return true; }
		bool isComment() override { return false; }

		bool isValidCommand() { return m_code.getOpCode() != CommandCodes::NoCommand; }

		void generateBinCode(ByteOrder byteOrder) override;

		QString getMnemoCode();

		int address() const { return m_address; }
	};


	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT
	private:
		QVector<CodeItem*> m_codeItems;

		int m_commandAddress = 0;

		ByteOrder m_byteOrder = ByteOrder::BigEndian;

	public:
		ApplicationLogicCode();
		~ApplicationLogicCode();

		void append(const Command &cmd);
		void append(const Comment &cmt);
		void comment(QString commentStr);
		void newLine();

		void clear();

		void generateBinCode();

		void getAsmCode(QStringList& asmCode);
		void getBinCode(QByteArray& byteArray);
		void getMifCode(QStringList& mifCode);

		void setByteOrder(ByteOrder byteOrder) { m_byteOrder = byteOrder; }
	};


}
