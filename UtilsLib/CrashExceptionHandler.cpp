#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "CrashExceptionHandler.h"

#if defined (Q_OS_WIN)

CrashExceptionHandler::CrashExceptionHandler(const QString& equipmentID):
	QObject()
{
    if (pThis != nullptr)
    {
        qDebug() << "CrashExceptionHandlerclass was already created!";

        Q_ASSERT(false);
        return;
    }

    pThis = this;

	m_equipmentID = equipmentID;

    SetUnhandledExceptionFilter(TopLevelExceptionHandler);
    //EnableDumping(10);
}

#pragma comment ( lib, "dbghelp.lib" )
#pragma comment ( lib, "user32.lib" )

bool CrashExceptionHandler::EnableDumping(DWORD dumpCount)
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

void CrashExceptionHandler::CreateMiniDump(EXCEPTION_POINTERS* pep)
{
	QString dumpFileName;

	if (m_equipmentID.isEmpty() == true)
	{
		dumpFileName = QCoreApplication::instance()->applicationName() + "_" +
					   DateTimeToString::fileName(QDateTime::currentDateTime()) + ".dmp";
	}
	else
	{
		dumpFileName = QCoreApplication::instance()->applicationName() + "_" +
					   m_equipmentID + "_" +
					   DateTimeToString::fileName(QDateTime::currentDateTime()) + ".dmp";
	}

	QString dumpPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if (QDir().exists(dumpPath) == false)
    {
        QDir().mkdir(dumpPath);
    }

    QString fullDumpFilePath = dumpPath + QDir::separator() + dumpFileName;

    HANDLE hFile = CreateFile(reinterpret_cast<LPCWSTR>(fullDumpFilePath.utf16()), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    bool result = false;

    if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		// Create the minidump

		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;

		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = pep;
		exceptionInfo.ClientPointers = FALSE;

		MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithFullMemory);

        result = MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			dumpType,
			(pep != NULL) ? &exceptionInfo : NULL,
			NULL,
			NULL
		);

        // Close the file

        CloseHandle(hFile);
    }

    emit miniDumpCreated(fullDumpFilePath, result);

	return;
}

LONG CrashExceptionHandler::TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    pThis->CreateMiniDump(pExceptionInfo);

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif
