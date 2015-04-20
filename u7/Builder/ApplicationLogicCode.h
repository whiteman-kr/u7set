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


#pragma pack(push, 1)

	struct CommandCode
	{
		union
		{
			struct
			{
				quint16 CRC5 : 5;
				quint16 code : 5;
				quint16 fbType : 6;
			} opCode;

			quint16 word1;
		};

		quint16 word2;

		union
		{
			struct
			{
				quint16 fbInstance : 10;
				quint16 fbParamNo : 6;

			} param;

			quint16 word3;
		};

		quint16 word4;
	};

#pragma pack(pop)


	class Command
	{
	private:
		QByteArray m_rawCode;
		QString m_asmCode;
		int address = -1;

	public:
		Command() {}

		void NOP();
		void START();
		void STOP();
		void MOV(quint16 addrFrom, quint16 addrTo);
		void MOVMEM(quint16 addrFrom, quint16 addrTo, quint16 sizeW);
		void MOVC(quint16 contVal, quint16 addrTo);
		void MOVBC(quint16 constBit, quint16 addrTo, quint16 bitNo);
		void WRFB(quint16 addrFrom, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void RDFB(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrTo);
		void WRFBC(quint16 constVal, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		void WRFBB(quint16 addrFrom, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo);
		//4,		//	RDFBB
	};


	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT
	private:
		QVector<Command> m_commands;

	public:
		ApplicationLogicCode();
	};


}
