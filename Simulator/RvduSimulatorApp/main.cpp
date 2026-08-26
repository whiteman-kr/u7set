#include "MainWindow.h"
#include "version.h"
#include <CommonLib/ConstStrings.h>
#include <CommonStdLib/u7_vld.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

static QString m_buildFolder;
static int m_subsystemKey = 0;
static QString m_sharedMemoryKey;

bool parseCommandLine()
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

	m_buildFolder = build;
	m_subsystemKey = key;
	m_sharedMemoryKey = memoryKey;

	return true;
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

	if (parseCommandLine() == false)
	{
		return -1;
	}

	MainWindow w(m_buildFolder, m_subsystemKey, m_sharedMemoryKey);
	w.show();

	return app.exec();
}
