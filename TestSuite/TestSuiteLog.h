#pragma once

#include "../UtilsLib/LogFile.h"
#include <TestSuiteLib/TestLog.h>
#include <queue>

class TestSuiteLogFile : public Log::LogFile
{
	Q_OBJECT

public:
	TestSuiteLogFile(const QString& fileName,
					 const QString& path,
					 int maxFileSize = 1048576,
					 int maxFilesCount = 64,
					 bool addAppInfoOnStart = true);

	bool writeAlert(const QString& text, const QString& tag = {}) override;
	bool writeError(const QString& text, const QString& tag = {}) override;
	bool writeWarning(const QString& text, const QString& tag = {}) override;
	bool writeMessage(const QString& text, const QString& tag = {}) override;
	bool writeText(const QString& text, const QString& tag = {}) override;
};

class TestSuiteTestLogOutput : public TestSuite::ITestLogOutput
{
public:
	TestSuiteTestLogOutput();

public:
	QString htmlFont() const;
	void setHtmlFont(QString fontName);

	bool queueIsEmpty() const;
	void pushQueue(const std::vector<TestSuite::TestLogItem>& in);
	void popQueue(std::vector<TestSuite::TestLogItem>* out, int maxCount);

private:
	void logItemArrived(const TestSuite::TestLogItem& item) override;

private:
	mutable QReadWriteLock m_lock;
	std::queue<TestSuite::TestLogItem> m_itemsQueue;
	QString m_htmlFont;
};