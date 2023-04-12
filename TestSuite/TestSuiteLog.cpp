#include "TestSuiteLog.h"

TestSuiteLogFile::TestSuiteLogFile(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount, bool addAppInfoOnStart)
	:Log::LogFile(fileName, path, maxFileSize, maxFilesCount, addAppInfoOnStart)
{

}

bool TestSuiteLogFile::writeAlert(const QString& text)
{
	emit errorArrived(text);
	return Log::LogFile::writeAlert(text);
}

bool TestSuiteLogFile::writeError(const QString& text)
{
	emit errorArrived(text);
	return Log::LogFile::writeError(text);
}

bool TestSuiteLogFile::writeWarning(const QString& text)
{
	emit warningArrived(text);
	return Log::LogFile::writeWarning(text);
}

bool TestSuiteLogFile::writeMessage(const QString& text)
{
	emit messageArrived(text);
	return Log::LogFile::writeMessage(text);
}

bool TestSuiteLogFile::writeText(const QString& text)
{
	emit textArrived(text);
	return Log::LogFile::writeText(text);
}

void TestSuiteTestLog::writeError(const QString& text)
{
	emit errorArrived(text);
}

void TestSuiteTestLog::writeWarning(const QString& text)
{
	emit warningArrived(text);
}

void TestSuiteTestLog::writeMessage(const QString& text)
{
	emit messageArrived(text);
}
