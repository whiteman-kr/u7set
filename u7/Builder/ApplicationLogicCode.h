#pragma once

#include <QObject>

namespace Builder
{

	enum CommandCode
	{
		NOP = 1,
		STOP = 2,
		MOV = 3,
		MOVARR = 4,
		MOVC = 5,
		WRFB = 6,
		RDFB = 7,
		WRFBC = 8,
		WRFBB = 9,
		RDFBB = 10
	};




	class ApplicationLogicCommand
	{
	private:
		QByteArray m_rawCode;
		QString m_asmCode;
		int address = -1;

	public:
		ApplicationLogicCommand() {}
	};


	class ApplicationLogicCode : public QObject
	{
		Q_OBJECT
	private:
		QVector<ApplicationLogicCommand> m_commands;

	public:
		ApplicationLogicCode();
	};


}
