#include <QCoreApplication>
#include <QDebug>
#include "../Simulator/Simulator.h"

static QtMessageHandler originalMessageHandler = 0;


void messageOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (QString(context.category) == QLatin1String("u7.sim"))
	{
		QByteArray localMsg = msg.toLocal8Bit();
		switch (type)
		{
		case QtDebugMsg:
			fprintf(stderr, "dbg: %s\n", localMsg.constData());
			break;
		case QtInfoMsg:
			fprintf(stderr, "inf: %s\n", localMsg.constData());
			break;
		case QtWarningMsg:
			fprintf(stderr, "wrn: %s\n", localMsg.constData());
			break;
		case QtCriticalMsg:
			fprintf(stderr, "err: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
			break;
		case QtFatalMsg:
			fprintf(stderr, "fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
			abort();
		}
	}
	else
	{
		originalMessageHandler(type, context, msg);
	}

	return;
}

int main(int argc, char *argv[])
{
	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication a(argc, argv);

	// --
	//
	Sim::Simulator simulator;

	bool ok = simulator.load("D:/Develop/build/test_simulator_bts-debug/build");
	if (ok == false)
	{
		return 1;
	}

	std::shared_ptr<Sim::LogicModule> logicModule = simulator.logicModule("SYSTEMID_RACKID_FSCC01_MD00");
	if (logicModule == nullptr)
	{
		return 2;
	}

	ok = logicModule->powerOn(true);
	if (ok == false)
	{
		return 3;
	}

	getc(stdin);

	return 0;
}
