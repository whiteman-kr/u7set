#include "MainWindow.h"
#include "version.h"
#include <CommonLib/ConstStrings.h>
#include <CommonStdLib/u7_vld.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QSharedMemory>

std::unique_ptr<QSharedMemory> theSharedMemorySingleApp;

bool parseCommandLine(AppSettings& settings)
{
	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	// A string option with id (-id)

	QCommandLineOption idOptionBuild("build", "RVDU folder build path");
	idOptionBuild.setValueName("build");
	parser.addOption(idOptionBuild);

	QCommandLineOption idOptionKey("key", "RVDU subsystem key");
	idOptionKey.setValueName("key");
	parser.addOption(idOptionKey);

	QCommandLineOption idMemoryKey("memory", "Simulator shared memory key");
	idMemoryKey.setValueName("memory");
	parser.addOption(idMemoryKey);

	parser.process(*qApp);

	QString build = parser.isSet(idOptionBuild) ? parser.value(idOptionBuild) : "";
	QString keyStr = parser.isSet(idOptionKey) ? parser.value(idOptionKey) : "";
	QString memoryKey = parser.isSet(idMemoryKey) ? parser.value(idMemoryKey) : "";

	bool keyOk = false;
	int key = keyStr.toInt(&keyOk);

	if (build.isEmpty() == true || keyStr.isEmpty() == true || keyOk == false)
	{
		QMessageBox::critical(
			nullptr,
			qAppName(),
			QObject::tr("usage: RvduSimulatorApp --build=D:\\WORK\\Builds\\mad_3_rvdu\\build\\Subsystems\\RC01 --key=24897 [--memory=VduSimulator.pid:0000.eid:0000]"));
		return false;
	}

	settings.buildFolder = build;
	settings.subsystemKey = key;
	settings.sharedMemoryKey = memoryKey;

	return true;
}

static bool checkRunningInstance(const QString& memoryId)
{
	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = std::make_unique<QSharedMemory>(QString("RvduSimulatorApp") + memoryId);

	if (theSharedMemorySingleApp->attach(QSharedMemory::ReadWrite) == false)
	{
		if (theSharedMemorySingleApp->create(sizeof(RvduSimSharedData)) == false)
		{
			qDebug() << "Failed to create QSharedMemory object!";
			assert(false);
			return false;
		}
		else
		{
			std::scoped_lock lock(*theSharedMemorySingleApp);
			auto sharedData = static_cast<RvduSimSharedData*>(theSharedMemorySingleApp->data());
			*sharedData = RvduSimSharedData{};
			return true;
		}
	}
	else
	{
		std::scoped_lock lock(*theSharedMemorySingleApp);
		auto sharedData = static_cast<RvduSimSharedData*>(theSharedMemorySingleApp->data());
		sharedData->showCommand = true;
		return false;
	}

	return false;
}

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	QApplication app(argc, argv);

	app.setApplicationName("RvduSimulatorApp");
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);
	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	AppSettings settings;

	if (parseCommandLine(settings) == false)
	{
		return -1;
	}

	bool runAppAllow = true;

	if (settings.sharedMemoryKey.isEmpty() == false)
	{
		if (checkRunningInstance(settings.sharedMemoryKey) == false) 
		{
			runAppAllow = false;
		}
	}

	int result = 0;

	if (runAppAllow == true)
	{
		MainWindow w(settings);
		w.show();

		result = app.exec();

	}

	return result;
}
