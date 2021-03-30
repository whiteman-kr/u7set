#include <QCoreApplication>
#include <QDebug>

// Visual Leak Detector
//
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
	#if __has_include("C:/Program Files (x86)/Visual Leak Detector/include/vld.h")
		#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
	#else
		#if __has_include("D:/Program Files (x86)/Visual Leak Detector/include/vld.h")
			#include "D:/Program Files (x86)/Visual Leak Detector/include/vld.h"
		#endif
	#endif
#endif	// Visual Leak Detector

#include "../Simulator/Simulator.h"

static QtMessageHandler originalMessageHandler = 0;


void messageOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (QString(context.category) == QLatin1String("u7.sim"))
	{
		QByteArray localMsg = msg.toLocal8Bit();
		switch (type)	// NOLINT
		{
#ifdef QT_DEBUG
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
	std::cout << "  SimulatorConsole [-build=build_dir] [-script=script_file] [-profile=profile_name]\n";
	std::cout << "\n";
	std::cout << "Create template simualtion script:\n";
	std::cout << "  SimulatorConsole [-create=file_name]\n";
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
	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication app(argc, argv);

	// Parse arguments
	// SimulatorConsole.exe [-build=build_dir] [-script=script_file] [-profile=profile_name]
	// SimulatorConsole.exe [-create=file_name]
	//
	QStringList args = QCoreApplication::arguments();

	if (args.size() < 2)
	{
		showProgrammUsageHint();
		return EXIT_FAILURE;
	}

	QString buildPath;
	QString scriptFile;
	QString profileName;

	for (int argIndex = 1; argIndex < argc; argIndex++)
	{
		if (args[argIndex].startsWith("-create=", Qt::CaseInsensitive) == true)
		{
			bool ok = generateScript(args[argIndex]);
			return ok == true ? EXIT_SUCCESS : EXIT_FAILURE;
		}

		if (args[argIndex].startsWith("-build=", Qt::CaseInsensitive) == true)
		{
			buildPath = args[argIndex];
			buildPath.replace("-build=", "", Qt::CaseInsensitive);
			continue;
		}

		if (args[argIndex].startsWith("-script=", Qt::CaseInsensitive) == true)
		{
			scriptFile = args[argIndex];
			scriptFile.replace("-script=", "", Qt::CaseInsensitive);
			continue;
		}

		if (args[argIndex].startsWith("-profile=", Qt::CaseInsensitive) == true)
		{
			profileName = args[argIndex];
			profileName.replace("-profile=", "", Qt::CaseInsensitive);
			continue;
		}

		// --
		//
		std::cout << "Unknown argument: " << args[argIndex].toStdString() << "\n\n";

		showProgrammUsageHint();
		return EXIT_FAILURE;
	}


	// --
	//
	Sim::Simulator simulator{nullptr, nullptr};		// Log to console

	if (bool ok = simulator.load(buildPath);
		ok == false)
	{
		return EXIT_FAILURE;
	}

	simulator.setCurrentProfile(profileName);

	// Add modules to simulation
	//
	simulator.control().setRunList({});

	bool ok = true;

	if (scriptFile.isEmpty() == false)
	{
		const qint64 timeout = 3600 * 1000;	// 1 hour, -1 means no time limit
		ok &= runScript(scriptFile, timeout, &simulator);
	}
	else
	{
		// Start timeless simulation till enter is pressed
		//
		ok &= simulator.control().startSimulation(std::chrono::microseconds{-1});
		getchar();
	}

	// Check if any LM after simulation is in failure mode
	//
	std::vector<std::shared_ptr<Sim::LogicModule>> lms = simulator.logicModules();

	for (auto lm : lms)
	{
		if (lm->deviceState() == Sim::DeviceState::Fault)
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
