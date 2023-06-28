
#include "main.h"
#include "TestSuiteMainWindow.h"

#include "AppConfigSettings.h"

#include <QApplication>

QSharedMemory* theSharedMemorySingleApp = nullptr;

#pragma pack(1)
struct TestSuiteSharedData
{
	int version = 1;
	bool showCommand = false;
};
#pragma pack()

QTranslator m_translator; // contains the translations for this application

void switchTranslator(QTranslator& translator, const QString& filename)
{
	// remove the old translator
	qApp->removeTranslator(&translator);

	// load the new translator
	if(translator.load(filename))
	{
		qApp->installTranslator(&translator);
	}
}

void loadLanguage(const QString& rLanguage)
{
	QLocale locale = QLocale(rLanguage);
	QLocale::setDefault(locale);

	switchTranslator(m_translator, QString(":/languages/TestSuite_%1.qm").arg(rLanguage));
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("TestSuite");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.9.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_COMMIT_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.9.LOCALBUILD"));
#endif

	int result = 0;

	theSettings.RestoreUser();
	theSettings.RestoreSystem();

	loadLanguage(theSettings.language());

	// Parse the command line
	//

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
		theSettings.librarySettings().setInstanceStrId(clientID);
	}

	//
	//


	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TestSuite, theSettings.librarySettings().instanceStrId(), 0, 1);

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TestSuite") + theSettings.librarySettings().instanceStrId());

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
			theMainWindow = new TestSuiteMainWindow(softwareInfo);
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
