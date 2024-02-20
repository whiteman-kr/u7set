#include "AppConfigSettings.h"
#include "TestSuiteMainWindow.h"
#include "version.h"
#include "../Protobuf/google/protobuf/message_lite.h"

#include <QApplication>
#include <QCommandLineParser>



QSharedMemory* theSharedMemorySingleApp = nullptr;

#pragma pack(1)
struct TestSuiteSharedData
{
	int version = 1;
	bool showCommand = false;
};
#pragma pack()

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("TestSuite");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(QString("%1.%2.%3 (%4)")
							.arg(U7SET_MAJOR_VERSION)
							.arg(U7SET_MINOR_VERSION)
							.arg(U7SET_PATCH_VERSION)
							.arg(U7SET_BRANCH_NAME));

	int result = 0;

	// Parse the command line
	//
	{
		QStringList arguments = a.arguments();

		QString settingsFileName;
		for (const QString& s : arguments)
		{
			if (s.contains(".ini") == true)
			{
				settingsFileName = s;
				break;
			}
		}

		if (settingsFileName.isEmpty() == false && QFile::exists(settingsFileName) == false)
		{
			QMessageBox::critical(nullptr, qAppName(), QObject::tr("Application settings file %1 is not exist.").arg(settingsFileName));
			return 1;
		}

		// Read settings
		//
		if (settingsFileName.isEmpty() == true)
		{
			AppConfigSettings::instance().load();
		}
		else
		{
			bool loadSettingsOk = AppConfigSettings::instance().loadFromFile(settingsFileName);
			if (loadSettingsOk == false)
			{
				QMessageBox::critical(nullptr, qAppName(), QObject::tr("Error loading application settings from file %1.").arg(settingsFileName));
				return 1;
			}
		}
	}

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	// A string option with id (-id)

	QCommandLineOption idOption("id", "Set the TestSuite ID.", "TestSuite ID");
	parser.addOption(idOption);

	parser.process(*qApp);

	QString clientID = parser.value(idOption);

	if (clientID.isEmpty() == false)
	{
		AppConfigSettings().instance().data().m_librarySettings.setInstanceStrId(clientID);
	}

	//
	//

	SoftwareInfo softwareInfo(E::SoftwareType::TestSuite, AppConfigSettings().instance().librarySettings().instanceStrId());

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TestSuite") + AppConfigSettings().instance().librarySettings().instanceStrId());

	if (theSharedMemorySingleApp->attach(QSharedMemory::ReadWrite) == false)
	{
		if (theSharedMemorySingleApp->create(sizeof(TestSuiteSharedData)) == false)
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
			theMainWindow = new TestSuiteMainWindow(softwareInfo);
			theMainWindow->show();

			result = a.exec();

			delete theMainWindow;
			theMainWindow = nullptr;
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

	// VFrame30::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
