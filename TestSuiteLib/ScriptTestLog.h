#pragma once

#include "TestLog.h"

class ScriptTestLog : public QObject
{
	Q_OBJECT
public:
	ScriptTestLog(TestLog* testLog);

public slots:
	void writeError(const QString& message);
	void writeWarning(const QString& message);
	void writeMessage(const QString& message);

private:
	TestLog* m_testLog = nullptr;
};
