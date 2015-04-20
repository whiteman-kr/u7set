#pragma once

#include <QObject>

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

		Count
	};

	const int CommandLen[static_cast<int>(CommandCodes::Count)] =
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
	};

	const int COMMAND_LEN_COUNT = sizeof(CommandLen) / sizeof(const int);


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
		void setFbInstance(quint16 fbInstance);
		void setFbParamNo(quint16 fbParamNo);

		void setWord2(quint16 value) { word2 = value; }
		void setWord3(quint16 value) { word3 = value; }
		void setWord4(quint16 value) { word4 = value; }

		void setBitNo(quint16 bitNo);

		int getSizeW();
	};


	class Command
	{
	private:
		QString m_asmCode;
		int m_address = -1;

		CommandCode m_code;
		QString m_comment;

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

		void setAddress(int address) { m_address = address; }
		void setComment(const QString& comment) { m_comment = comment; }

		int getSizeW() { return m_code.getSizeW(); }
	};


	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT
	private:
		QVector<Command> m_commands;

		int m_commandAddress = 0;

	public:
		ApplicationLogicCode();

		void append(Command &cmd);

		void clear();
	};


}
