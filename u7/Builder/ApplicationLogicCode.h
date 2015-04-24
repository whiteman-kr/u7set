#pragma once

#include <QObject>
#include "../include/Types.h"

namespace Builder
{

	enum CommandCodes
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

		Count
	};

	const int COMMAND_COUNT = static_cast<int>(CommandCodes::Count);

	const int CommandLen[COMMAND_COUNT] =
	{
		0,		//	NoCommand
		1,		//	NOP
		1,		//	START
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


	const quint16	MAX_FB_TYPE = 64 - 1,
					MAX_FB_INSTANCE = 1024 - 1,
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
				quint16 CRC5 : 5;
				quint16 code : 5;
				quint16 fbType : 6;
			} opCode;

			quint16 word1 = 0;
		};

		quint16 word2 = 0;

		union
		{
			struct
			{
				quint16 fbInstance : 10;
				quint16 fbParamNo : 6;

			} param;

			quint16 word3 = 0;
		};

		quint16 word4 = 0;

#pragma pack(pop)

	public:
		CommandCode()
		{
			setNoCommand();
		}

		void setNoCommand() { opCode.code = CommandCodes::NoCommand; }

		void setOpCode(CommandCodes code);
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

		quint16 getWord(int index);

		int getSizeW();
	};


	class CodeItem
	{
	private:
		QString m_comment;

	public:
		virtual ~CodeItem();

		virtual QString toString() = 0;
		virtual int getSizeW()= 0;

		virtual bool isCommand() = 0;
		virtual bool isComment() = 0;

		virtual void generateBinCode(ByteOrder byteOrder) = 0;
		virtual QByteArray getBinCode() = 0;

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
		int getSizeW() override { return 0; }

		void generateBinCode(ByteOrder) override {}
		QByteArray getBinCode() override { return QByteArray(); }

		bool isCommand() override { return false; }
		bool isComment() override { return true; }
	};


	class Command : public CodeItem
	{
	private:
		int m_address = -1;

		CommandCode m_code;

		QByteArray m_binCode;

		QString getCodeWordStr(int wordNo);

		QString getMnemoCode();

	public:
		Command() {}

		void nop();
		void start();
		void stop();
		void mov(quint16 addrFrom, quint16 addrTo);
		void movMem(quint16 addrFrom, quint16 addrTo, quint16 sizeW);
		void movConst(quint16 constVal, quint16 addrTo);
		void movBitConst(quint16 constBit, quint16 addrTo, quint16 bitNo);
		void writeFuncBlock(quint16 addrFrom, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void readFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrTo);
		void writeFuncBlockConst(quint16 constVal, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void writeFuncBlockBit(quint16 addrFrom, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void readFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrTo, quint16 bitNo);
		void readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue);

		void setAddress(int address) { m_address = address; }

		QString toString() override;
		int getSizeW() override { return m_code.getSizeW(); }

		bool isCommand() override { return true; }
		bool isComment() override { return false; }

		void generateBinCode(ByteOrder byteOrder) override;
		QByteArray getBinCode() override { return m_binCode; }
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

		void clear();

		void generateBinCode();

		void getAsmCode(QStringList& asmCode);
		void getBinCode(QByteArray& byteArray);

		void setByteOrder(ByteOrder byteOrder) { m_byteOrder = byteOrder; }
	};


}
