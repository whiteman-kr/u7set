#include <QCoreApplication>
#include <QDebug>

// For detecting memory leaks
//
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif


#include "../Simulator/Simulator.h"

static QtMessageHandler originalMessageHandler = 0;


void messageOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (QString(context.category) == QLatin1String("u7.sim"))
	{
		QByteArray localMsg = msg.toLocal8Bit();
		switch (type)	// NOLINT
		{
#ifdef Q_DEBUG
		case QtDebugMsg:
			fprintf(stderr, "dbg: %s\n", localMsg.constData());
			break;
#endif
		case QtInfoMsg:
			fprintf(stderr, "inf: %s\n", localMsg.constData());
			break;
		case QtWarningMsg:
			fprintf(stderr, "wrn: %s\n", localMsg.constData());
			break;
		case QtCriticalMsg:
            fprintf(stderr, "err: %s\n", localMsg.constData());
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

void showProgrammUsageHint()
{
	std::cout << "Programm usage:\n";
	std::cout << "  SimulatorConsole <BuildDirectory>             - Run simulation for all modules from build <BuildDirectory>\n";
	std::cout << "  SimulatorConsole <BuildDirectory> [Script.js] - Run simulation script for build <BuildDirectory>\n";
	std::cout << "  SimulatorConsole [/create Script.js]          - Create simple example of simulation script\n";
	std::cout << "  SimulatorConsole [-create Script.js]          - Create simple example of simulation script\n";

	return;
}

bool generateScript(QString fileName)
{
	QFile file{fileName};
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		std::cout << file.errorString().toStdString() << "\n";
		return false;
	}

	QFile rcFile{":/ScriptSample.js"};
	if (rcFile.open(QIODevice::ReadOnly) == false)
	{
		std::cout << rcFile.errorString().toStdString() << "\n";
		return false;
	}

	QString str = rcFile.readAll();

	QTextStream out(&file);
	out << str;

	return true;
}

bool runScript(QString scriptFileName, qint64 timeout, Sim::Simulator* simulator)
{
	assert(simulator);

	// Script must be run
	//
	QFile file{scriptFileName};

	if (file.open(QIODevice::ReadOnly) == false)
	{
		qDebug() << file.errorString();
		return false;
	}

	QString script = file.readAll();

	bool ok = simulator->runScript({script, QFileInfo(file).baseName()}, timeout);
	if (ok == false)
	{
		return false;
	}

	ok = simulator->waitScript(timeout < 0 ? ULONG_MAX : timeout);
	if (ok == false)
	{
		return false;
	}

	ok = simulator->scriptResult();
	return ok;
}


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined(Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// To see all memory leaks, not only in the own code, comment the next line
	prevHook = _CrtSetReportHook(reportingHook);
#endif

	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication a(argc, argv);

	// --
	//
	QStringList args = QCoreApplication::arguments();

	if (args.size() < 2)
	{
		showProgrammUsageHint();
		return EXIT_FAILURE;
	}

	if (args[1].compare("/create", Qt::CaseInsensitive) == 0 ||
		args[1].compare("-create", Qt::CaseInsensitive) == 0)
	{
		if (args.size() < 3)
		{
			std::cout << " Script file name is not specified.\n";
			return EXIT_FAILURE;
		}

		bool ok = generateScript(args[2]);

		return ok == true ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	// --
	//
	Sim::Simulator simulator{nullptr, nullptr};		// Log to console
	QString buildPath = args[1];

	if (bool ok = simulator.load(buildPath);
		ok == false)
	{
		return EXIT_FAILURE;
	}

	// Add modules to simulation
	//
	simulator.control().setRunList({});

	bool ok = true;

	if (args.size() > 2)
	{
		// Script must be run
		//
		QString scriptFileName = args[2];
		qint64 timeout = 3600 * 1000;	// 1 hour, -1 means no time limit

		ok &= runScript(scriptFileName, timeout, &simulator);
	}
	else
	{
		// Start simulation
		//
		ok &= simulator.control().startSimulation(std::chrono::microseconds{-1});
		getchar();
	}

	// Check if any LM after simulation is in failure mode
	//
	std::vector<std::shared_ptr<Sim::LogicModule>> lms = simulator.logicModules();

	for (auto lm : lms)
	{
		if (lm->deviceMode() == Sim::DeviceMode::Fault)
		{
			QString message = QString("Simulation afterrun check: LogicModule %1 is in FAULT mode").arg(lm->equipmentId());
			std::cout << message.toStdString() << "\n";
			ok = false;
			break;
		}
	}

	// result
	//
	if (ok == false)
	{
		std::cout << "FAILED\n";
	}

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
