#pragma once

#include "TestLog.h"

class TestLogController : public QObject
{
	Q_OBJECT
public:
	TestLogController(TestLog *testLog);

public slots:
	void addMessage(const QString& message);
	void writeError(const QString& message);

private:
	TestLog* m_testLog = nullptr;
};
