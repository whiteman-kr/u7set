
#include "main.h"
#include "TestSuiteMainWindow.h"

#include "../OnlineLib/SoftwareInfo.h"
#include "Settings.h"

#include <QApplication>

QSharedMemory* theSharedMemorySingleApp = nullptr;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("TuningClient");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.9.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.9.LOCALBUILD"));
#endif

	int result = 0;

	theSettings.RestoreUser();
	theSettings.RestoreSystem();

	//loadLanguage(theSettings.language());

	// Parse the command line
	//


	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	// A string option with id (-id)

	QCommandLineOption idOption("id", "Set the TestSuite ID.", "TestSuite ID");
	parser.addOption(idOption);

	// A boolean option with simulation (-simulate)

	parser.process(*qApp);

	QString clientID = parser.value(idOption);

	if (clientID.isEmpty() == false)
	{
		theSettings.setInstanceStrId(clientID);
	}

	//
	//


	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TuningClient, theSettings.instanceStrId(), 0, 1);

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TestSuite") + theSettings.instanceStrId());

	if(theSharedMemorySingleApp->attach(QSharedMemory::ReadWrite) == false)
	{
		if(theSharedMemorySingleApp->create(sizeof(TestSuiteSharedData)) == false)
		{
			qDebug() << "Failed to create QSharedMemory object!";
			assert(false);
		}
		else
		{
			bool ok = theSharedMemorySingleApp->lock();
			if (ok == true)
			{
				void* buffer = theSharedMemorySingleApp->data();

				TestSuiteSharedData data;
				memcpy(buffer, &data, sizeof(TestSuiteSharedData));

				ok = theSharedMemorySingleApp->unlock();
				if (ok == false)
				{
					qDebug() << "Failed to unlock QSharedMemory object!";
					assert(false);
				}
			}
			else
			{
				qDebug() << "Failed to lock QSharedMemory object!";
				assert(false);
			}

			// Run the application
			//
			theMainWindow = new TestSuiteMainWindow(/*softwareInfo*/);
			theMainWindow->show();

			result = a.exec();

			delete theMainWindow;
			theMainWindow = nullptr;

			theSettings.StoreUser();
		}
	}
	else
	{
		QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Application is already running!"));

		bool ok = theSharedMemorySingleApp->lock();
		if (ok == true)
		{
			TestSuiteSharedData* data = (TestSuiteSharedData*)theSharedMemorySingleApp->data();

			data->showCommand = true;

			ok = theSharedMemorySingleApp->unlock();
			if (ok == false)
			{
				qDebug() << "Failed to unlock QSharedMemory object!";
				assert(false);
			}
		}
		else
		{
			qDebug() << "Failed to lock QSharedMemory object!";
			assert(false);
		}


		theSharedMemorySingleApp->detach();
	}

	if (theSharedMemorySingleApp != nullptr)
	{
		delete theSharedMemorySingleApp;
		theSharedMemorySingleApp = nullptr;
	}

	//VFrame30::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
