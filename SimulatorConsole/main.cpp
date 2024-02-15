#include <atomic>
#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFloat16>
#include <QTextStream>
#include <QVector>

#include "../Protobuf/google/protobuf/message_lite.h"
#include "../Simulator/SimConsoleLogFile.h"
#include "../Simulator/Simulator.h"
#include "../lib/ConstStrings.h"

static QtMessageHandler originalMessageHandler = 0;

std::atomic<bool> g_verbose = false;


void messageOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (QString(context.category) == QLatin1String("u7.sim"))
	{
		QByteArray localMsg = msg.toLocal8Bit();
		switch (type)
		{
		case QtDebugMsg:
			if (g_verbose.load(std::memory_order_relaxed) == true)
			{
				fprintf(stderr, "dbg: %s\n", localMsg.constData());
			}
			break;
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
			fprintf(stderr, "fatal: %s (%s:%d, %s)\n", localMsg.constData(), context.file, context.line, context.function);
			abort();
		}
	}
	else
	{
		originalMessageHandler(type, context, msg);
	}

	return;
}

void showProgramUsageHint()
{
	std::cout << "Program usage:\n";
	std::cout << "  SimulatorConsole [-build=build_dir] [-script=file] [-global_script=file] [-profile=profile_name] [-speed_factor=x0.1|x0.25|x0.5|x1|x2|x4|FF] [-verbose] [-enable_lan]\n";
	std::cout << "\n";
	std::cout << "Create template simulation script:\n";
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

bool runScript(QString scriptFileName, QString globalScriptFileName, qint64 timeout, Sim::Simulator* simulator)
{
	assert(simulator);

	// Script must be run
	//
	QFile scriptFile{scriptFileName};

	if (scriptFile.open(QIODevice::ReadOnly) == false)
	{
		std::cout << "Cannot open file: " << scriptFile.fileName().toStdString() << "\n";
		std::cout << scriptFile.errorString().toStdString() << "\n";
		return false;
	}

	Sim::SimScriptItem script{scriptFile.readAll(), QFileInfo(scriptFile).baseName()};

	// Attempt to Load global script.
	//
	Sim::SimScriptItem globalScript{};

	if (globalScriptFileName.isEmpty() == false)
	{
		// There is a global script file from arguments.
		//
		QFile globalScriptFile{globalScriptFileName};
		if (globalScriptFile.open(QIODevice::ReadOnly) == false)
		{
			std::cout << "Cannot open file: " << globalScriptFile.fileName().toStdString() << "\n";
			std::cout << globalScriptFile.errorString().toStdString() << "\n";
			return false;
		}

		globalScript = {globalScriptFile.readAll(), QFileInfo(globalScriptFile).baseName()};
	}
	else
	{
		// Global script was not passed as argument, try to load it from the same folder as script.
		//
		QFileInfo globalScriptFileInfo{QFileInfo{scriptFile}.absolutePath(), File::GLOBAL_SCRIPT};

		if (globalScriptFileInfo.exists() == true)
		{
			QFile globalScriptFile{globalScriptFileInfo.absoluteFilePath()};

			if (globalScriptFile.open(QIODevice::ReadOnly) == false)
			{
				std::cout << "Cannot open file: " << globalScriptFile.fileName().toStdString() << "\n";
				std::cout << globalScriptFile.errorString().toStdString() << "\n";
				return false;
			}

			globalScript = {globalScriptFile.readAll(), QFileInfo(globalScriptFile).baseName()};
		}
		else
		{
			std::cout << "WARNING: File " << File::GLOBAL_SCRIPT.toStdString() << " is not loaded." << "\n";
		}
	}

	bool ok = simulator->runScript(script, globalScript, timeout);
	if (ok == false)
	{
		return false;
	}

	ok = simulator->waitScript(static_cast<unsigned long>(timeout < 0 ? ULONG_MAX : timeout));
	if (ok == false)
	{
		return false;
	}

	ok = simulator->scriptResult();
	return ok;
}

class ProtobufLibShutdowner
{
public:
	~ProtobufLibShutdowner()
	{
		google::protobuf::ShutdownProtobufLibrary();
	}
};

int main(int argc, char* argv[])
{
	ProtobufLibShutdowner pbLibShutdowner;
	Q_UNUSED(pbLibShutdowner);

	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication app(argc, argv);

	// Parse arguments
	//
	QStringList args = QCoreApplication::arguments();

	if (args.size() < 2)
	{
		showProgramUsageHint();
		return EXIT_FAILURE;
	}

	QString buildPath;
	QString scriptFile;
	QString globalScriptFile;
	QString profileName;
	QString speedFactorStr;
	bool unlockTimer = false;
	bool enableLan = false;
	bool noExit = false;

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

		if (args[argIndex].startsWith("-global_script=", Qt::CaseInsensitive) == true)
		{
			globalScriptFile = args[argIndex];
			globalScriptFile.replace("-global_script=", "", Qt::CaseInsensitive);
			continue;
		}

		if (args[argIndex].startsWith("-profile=", Qt::CaseInsensitive) == true)
		{
			profileName = args[argIndex];
			profileName.replace("-profile=", "", Qt::CaseInsensitive);
			continue;
		}

		if (args[argIndex].compare("-unlock_timer", Qt::CaseInsensitive) == 0)
		{
			unlockTimer = true;
			continue;
		}

		if (args[argIndex].startsWith("-speed_factor=", Qt::CaseInsensitive) == true)
		{
			speedFactorStr = args[argIndex];
			speedFactorStr.replace("-speed_factor=", "", Qt::CaseInsensitive);
			continue;
		}

		if (args[argIndex].compare("-verbose", Qt::CaseInsensitive) == 0)
		{
			g_verbose.store(true);
			continue;
		}

		if (args[argIndex].compare("-enable_lan", Qt::CaseInsensitive) == 0)
		{
			enableLan = true;
			continue;
		}

		if (args[argIndex].compare("-no_exit", Qt::CaseInsensitive) == 0)
		{
			noExit = true;
			continue;
		}

		// --
		//
		std::cout << "Unknown argument: " << args[argIndex].toStdString() << "\n\n";

		showProgramUsageHint();
		return EXIT_FAILURE;
	}

	double speedFactor = 1.0;
	if (unlockTimer == true)
	{
		// -unlock_timer - is obsolete.
		//
		speedFactor = 256.0;
	}

	if (speedFactorStr.isEmpty() == false)
	{
		std::map<QString, double> speedFactorStrToValue{
			{".1", 0.1},
			{".25", 0.25},
			{".5", 0.5},
			{".75", 0.75},
			{"1", 1.0},
			{"2", 2.0},
			{"4", 4.0},
			{"8", 8.0},
			{"10", 10.0},
			{"16", 16.0},
			{"FF", 256.0}};

		speedFactorStr.remove("x", Qt::CaseInsensitive);
		if (speedFactorStr.startsWith("0.", Qt::CaseInsensitive) == true)
		{
			speedFactorStr.replace("0.", ".");
		}

		if (speedFactorStrToValue.contains(speedFactorStr) == true)
		{
			speedFactor = speedFactorStrToValue[speedFactorStr];
		}
		else
		{
			std::cout << "Wrong SpeedFactor: " << speedFactorStr.toLocal8Bit().data() << ", fallback to SpeedFactor 1.0\n";
		}
	}

	if (speedFactor >= 256.0)
	{
		std::cout << "SpeedFactor: FF\n";
	}
	else
	{
		std::cout << "SpeedFactor: " << speedFactor << "\n";
	}

	// --
	//
	Sim::ConsoleLogFile consoleLog;
	Sim::Simulator simulator{&consoleLog, g_verbose, nullptr}; // Log to console

	if (bool ok = simulator.load(buildPath);
		ok == false)
	{
		return EXIT_FAILURE;
	}

	simulator.setCurrentProfile(profileName);
	simulator.control().setSpeedFactor(speedFactor);
	simulator.control().setRunList({}); // Add all modules to simulation
	simulator.software().setEnabled(enableLan);

	bool ok = true;

	if (scriptFile.isEmpty() == false)
	{
		const qint64 timeout = 3600 * 1000; // 1 hour, -1 means no time limit
		ok &= runScript(scriptFile, globalScriptFile, timeout, &simulator);
	}
	else
	{
		// Start timeless simulation till enter is pressed
		//
		ok &= simulator.control().startSimulation(std::chrono::microseconds{-1});

		if (noExit == true)
		{
			// Forever sleep
			//
			std::promise<void>().get_future().wait();
		}
		else
		{
			// Wait for Enter
			//
			std::cout << "Press Enter to stop simulation and exit...\n";
			std::string str;
			std::getline(std::cin, str);
		}
	}

	// Check if any LM after simulation is in failure mode
	//
	std::vector<std::shared_ptr<Sim::LogicModule>> lms = simulator.logicModules();

	for (const auto& lm : lms)
	{
		if (lm->deviceState() == Sim::DeviceState::Fault)
		{
			QString message = QString("Simulation after-run check: LogicModule %1 is in FAULT mode").arg(lm->equipmentId());
			std::cout << message.toStdString() << "\n";
			ok = false;
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
