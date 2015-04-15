#pragma once

#include <QObject>


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


