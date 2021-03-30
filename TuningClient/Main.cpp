#include "Main.h"
#include "MainWindow.h"
#include <QApplication>
#include "Settings.h"
#include "UserManager.h"
#include <QCommandLineParser>
#include "../VFrame30/VFrame30Library.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

//// ---------------- Minidump generating functions -------------------
////
#if defined (Q_OS_WIN)

#pragma comment ( lib, "dbghelp.lib" )
#pragma comment ( lib, "user32.lib" )

#include <windows.h>
#include <processthreadsapi.h>
#include <fileapi.h>
#include <dbghelp.h>

void CreateMiniDump(EXCEPTION_POINTERS* pep);

BOOL CALLBACK MyMiniDumpCallback(
	PVOID                            pParam,
	const PMINIDUMP_CALLBACK_INPUT   pInput,
	PMINIDUMP_CALLBACK_OUTPUT        pOutput
);

void CreateMiniDump(EXCEPTION_POINTERS* pep)
{
	QString dumpFileName = qAppName() + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss") + ".dmp";

	QString dumpPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	QString fullDumpPath = dumpPath + QDir::separator() + dumpFileName;

	HANDLE hFile = CreateFile(reinterpret_cast<LPCWSTR>(fullDumpPath.utf16()), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	HWND hWnd = NULL;
	if (theMainWindow != nullptr)
	{
		hWnd = reinterpret_cast<HWND>(theMainWindow->winId());
	}

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		// Create the minidump

		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;

		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = pep;
		exceptionInfo.ClientPointers = FALSE;

		MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithFullMemory);

		BOOL result = MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			dumpType,
			(pep != NULL) ? &exceptionInfo : NULL,
			NULL,
			NULL
		);

		if (result == false)
		{
			MessageBoxW(hWnd, L"Application has been crashed!\n\nCrash dump creating failed.", reinterpret_cast<LPCWSTR>(qAppName().utf16()), MB_OK|MB_ICONERROR);
		}
		else
		{
			QString message = QObject::tr("Application has been crashed!\n\nA crash dump has been created:\n\n%1\n\nPlease send this file and program execulable file to support.").arg(fullDumpPath);

			MessageBoxW(hWnd, reinterpret_cast<LPCWSTR>(message.utf16()), reinterpret_cast<LPCWSTR>(qAppName().utf16()), MB_OK | MB_ICONERROR);
		}

		// Close the file

		CloseHandle(hFile);
	}
	else
	{
		QString message = QObject::tr("Application has been crashed!\n\nColld not save crash dump file:\n\n%1.").arg(fullDumpPath);

		MessageBoxW(hWnd, reinterpret_cast<LPCWSTR>(message.utf16()), reinterpret_cast<LPCWSTR>(qAppName().utf16()), MB_OK | MB_ICONERROR);
	}

	return;
}


LONG TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	CreateMiniDump(pExceptionInfo);

	return EXCEPTION_CONTINUE_SEARCH;
}

bool EnableDumping(DWORD dumpCount)
{
	HKEY k;

	TCHAR ModuleName[4096];
	GetModuleFileName(NULL, ModuleName, 4096);

	QString ApplicationPath =  QString::fromWCharArray(ModuleName);

	QString dumpFolder = QObject::tr("%LOCALAPPDATA%\\CrashDumps");

	DWORD dwDumpType = 2;

	QString keyName = QObject::tr("SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps\\%1").arg(ApplicationPath.mid(ApplicationPath.lastIndexOf('\\') + 1));

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, reinterpret_cast<LPCWSTR>(keyName.utf16()), 0, KEY_READ, &k) != ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, reinterpret_cast<LPCWSTR>(keyName.utf16()), 0, NULL, 0, KEY_WRITE | KEY_WOW64_64KEY, NULL, &k, NULL) != ERROR_SUCCESS
			|| RegSetValueEx(k, L"DumpCount", NULL, REG_DWORD, (BYTE*)&dumpCount, sizeof(dumpCount)) != ERROR_SUCCESS
			|| RegSetValueEx(k, L"DumpFolder", NULL, REG_EXPAND_SZ, (BYTE*)dumpFolder.utf16(), sizeof(TCHAR) * (keyName.length() + 1)) != ERROR_SUCCESS
			|| RegSetValueEx(k, L"DumpType", NULL, REG_DWORD, (BYTE*)&dwDumpType, sizeof(dwDumpType)) != ERROR_SUCCESS)
		{
			return false;
		}
	}

	RegCloseKey(k);

	return true;
}

#endif

////
//// ---------------- Minidump generating functions -------------------

// ---------------- Translator functions -------------------
//

QTranslator m_translator; // contains the translations for this application

QSharedMemory* theSharedMemorySingleApp = nullptr;

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

	switchTranslator(m_translator, QString(":/languages/TuningClient_%1.qm").arg(rLanguage));
}

//
// ---------------- Translator functions -------------------


int main(int argc, char* argv[])
{

#if defined (Q_OS_WIN)
	// Set writing minidumps handler
	//
	SetUnhandledExceptionFilter(TopLevelExceptionHandler);

	EnableDumping(10);
#endif

	int result = 0;

	QApplication a(argc, argv);
	a.setApplicationName("TuningClient");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");


#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.8.LOCALBUILD"));
#endif

	VFrame30::init();

	theSettings.RestoreUser();
	theSettings.RestoreSystem();

	loadLanguage(theSettings.language());

	// Parse the command line
	//

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	// A string option with id (-id)

	QCommandLineOption idOption("id", "Set the TuningClient ID.", "TuningClient ID");
	parser.addOption(idOption);

	// A boolean option with simulation (-simulate)

	QCommandLineOption simulationOption("simulate", "Simulate signals values");
	if (theSettings.m_enableSimulation == true)
	{
		parser.addOption(simulationOption);
	}

	parser.process(*qApp);

	QString clientID = parser.value(idOption);

	if (clientID.isEmpty() == false)
	{
	    theSettings.setInstanceStrId(clientID);
	}

	if (theSettings.m_enableSimulation == true)
	{
		theSettings.m_simulationMode = parser.isSet(simulationOption);
	}

	//
	//

	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TuningClient, theSettings.instanceStrId(), 0, 1);

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TuningClient") + theSettings.instanceStrId());

	if(theSharedMemorySingleApp->attach(QSharedMemory::ReadWrite) == false)
	{
		if(theSharedMemorySingleApp->create(sizeof(TuningClientSharedData)) == false)
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

				TuningClientSharedData data;
				memcpy(buffer, &data, sizeof(TuningClientSharedData));

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
			theMainWindow = new MainWindow(softwareInfo);
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
			TuningClientSharedData* data = (TuningClientSharedData*)theSharedMemorySingleApp->data();

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

	delete theSharedMemorySingleApp;
	theSharedMemorySingleApp = nullptr;

	VFrame30::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
