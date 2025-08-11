#include "Main.h"

#include <CommonLib/u7_vld.h>
#include <ClientLib/TuningUserManager.h>
#include <VFrame30/VFrame30Library.h>
#include <UiLib/OverrideWindows11Style.h>

#include "MainWindow.h"
#include "ScriptTuningClientApplication.h"
#include "Settings.h"
#include "version.h"

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
	if (theApp.mainWindow() != nullptr)
	{
		hWnd = reinterpret_cast<HWND>(theApp.mainWindow()->winId());
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
			|| RegSetValueEx(k, L"DumpFolder", NULL, REG_EXPAND_SZ, (BYTE*)dumpFolder.utf16(), sizeof(TCHAR) * static_cast<DWORD>(keyName.length() + 1)) != ERROR_SUCCESS
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

QSharedMemory* theSharedMemorySingleApp = nullptr;
ScriptTuningClientApplication theApp;

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

#if defined (Q_OS_WIN)
	// Set writing minidumps handler
	//
	SetUnhandledExceptionFilter(TopLevelExceptionHandler);

	EnableDumping(10);
#endif

	int result = 0;

	QApplication a(argc, argv);

	UiLib::OverrideWindows11Style(a, argc, argv);

	a.setApplicationName("TuningClient");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(QString("%1.%2.%3 (%4)")
							.arg(U7SET_MAJOR_VERSION)
							.arg(U7SET_MINOR_VERSION)
							.arg(U7SET_PATCH_VERSION)
							.arg(U7SET_BRANCH_NAME));

	VFrame30::init();

	TuningClientAppSettings::instance().loadUser();

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
			TuningClientAppSettings::instance().load();
		}
		else
		{
			bool loadSettingsOk = TuningClientAppSettings::instance().loadFromFile(settingsFileName);
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
	//
	QCommandLineOption idOption("id", "Set the TuningClient ID.", "TuningClient ID");
	parser.addOption(idOption);

	parser.process(*qApp);

	QString clientID = parser.value(idOption);

	if (clientID.isEmpty() == false)
	{
		TuningClientAppSettings::instance().system().m_instanceStrId = clientID;
	}

	//
	//

	SoftwareInfo softwareInfo(E::SoftwareType::TuningClient, TuningClientAppSettings::instance().instanceStrId());

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TuningClient") + TuningClientAppSettings::instance().instanceStrId());

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
			{
				MainWindow mainWindow(softwareInfo);

				theApp.setMainWindow(&mainWindow);

				mainWindow.show();

				result = a.exec();

				theApp.setMainWindow(nullptr);
			}

			TuningClientAppSettings::instance().saveUser();
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

	if (theSharedMemorySingleApp != nullptr)
	{
		delete theSharedMemorySingleApp;
		theSharedMemorySingleApp = nullptr;
	}

	VFrame30::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
