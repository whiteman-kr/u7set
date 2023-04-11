#pragma once

#include "../UtilsLib/LogFile.h"
#include "../TestSuiteLib/TestLog.h"

class TestSuiteLogFile : public Log::LogFile
{
	Q_OBJECT
public:
	TestSuiteLogFile(const QString& fileName, const QString& path, int maxFileSize = 1048576, int maxFilesCount = 64, bool addAppInfoOnStart = true);

	bool writeAlert(const QString& text) override;
	bool writeError(const QString& text) override;
	bool writeWarning(const QString& text) override;
	bool writeMessage(const QString& text) override;
	bool writeText(const QString& text) override;

signals:
	void errorArrived(const QString& text);
	void warningArrived(const QString& text);
	void messageArrived(const QString& text);
	void textArrived(const QString& text);
};

class TestSuiteOutputLog : public QObject, public TestSuite::ITestLog
{
	Q_OBJECT
public:
	void writeError(const QString& text) override;
	void writeWarning(const QString& text) override;
	void writeMessage(const QString& text) override;

signals:
	void errorArrived(const QString& text);
	void warningArrived(const QString& text);
	void messageArrived(const QString& text);
};

