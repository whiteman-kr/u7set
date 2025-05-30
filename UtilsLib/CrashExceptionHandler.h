#pragma once

#include <QObject>

#if defined (Q_OS_WIN)

#include <windows.h>
#include <processthreadsapi.h>
#include <fileapi.h>
#include <dbghelp.h>

class CrashExceptionHandler : public QObject
{
    Q_OBJECT

public:
	CrashExceptionHandler(const QString& equipmentID = QString());

    bool EnableDumping(DWORD dumpCount);

signals:
    void miniDumpCreated(QString dumpFilePath, bool result);

private:
    static LONG TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);
    void CreateMiniDump(EXCEPTION_POINTERS* pep);

private:
	inline static QString m_equipmentID;
	inline static CrashExceptionHandler* pThis = nullptr;
};

#endif
